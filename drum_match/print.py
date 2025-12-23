#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import annotations

import os
import argparse
import numpy as np
from mido import MidiFile
from typing import Tuple

def match_best_from_cache(midi_path: str,
                          cache_path: str = "library_cache.npz",
                          weights: Tuple[float, float] = (0.6, 0.4),
                          return_score: bool = False,
                          verbose: bool = True):
    """
    입력 MIDI(항상 2마디 기준)를 캐시(library_cache.npz)에 들어있는 베이직 패턴들과 비교하여
    가장 유사한 파일명을 반환. verbose=True일 때는 01 시퀀스(K/S)를 분리해 보기 좋게 출력.

    Returns
    -------
    str | (str, float)
        가장 유사한 파일명 문자열 또는 (파일명, 유사도[0~1]) 튜플.

    Raises
    ------
    FileNotFoundError : cache_path 또는 midi_path가 없을 때
    ValueError        : 캐시 레이아웃이 킥/스네어(64차원)과 다를 때
    """
    BAR_LEN_BEATS = 4.0
    STEPS_PER_BAR = 16
    TARGET_BARS   = 2

    # ── 노트 매핑 확장: 킥/스네어가 38/40만이 아닌 경우까지 커버 ──
    KICK_NOTES  = {35, 36}              # BD
    SNARE_NOTES = {37, 38, 39, 40}      # SS/SD/HC/SD-rim 등 포함

    # ── 보조: 32칸(2마디=32스텝)을 가독성 있게 출력 ──
    def _fmt_32(vec32: np.ndarray) -> str:
        """
        32칸을 4칸씩 묶고, 16칸마다 바 구분. 예: '1000 0000 1000 0000 | 1000 0000 1000 0000'
        """
        s = "".join(map(str, vec32.tolist()))
        chunks = [s[i:i+4] for i in range(0, 32, 4)]  # 4칸씩
        left  = " ".join(chunks[0:4])   # 0~15 중 0~15의 0~15? (4묶음=16칸)
        left += " " + " ".join(chunks[4:8])
        right = " ".join(chunks[8:12])  # 16~31
        right += " " + " ".join(chunks[12:16])
        return f"{left} | {right}"

    def _pretty_print_ks(tag: str, vec64: np.ndarray):
        k = vec64[:32]
        s = vec64[32:]
        print(f"\n=== {tag} ===")
        print(f"[KICK ] {_fmt_32(k)}   (hits={int(k.sum())})")
        print(f"[SNARE] {_fmt_32(s)}   (hits={int(s.sum())})")

    # ── MIDI → 0/1 그리드(바 리스트: [{'kick':[16], 'snare':[16]}, ...]) ──
    def _midi_to_grids_01(_midi_path: str, velocity_min: int = 1, max_bars: int = TARGET_BARS):
        mid = MidiFile(_midi_path)
        tpq = mid.ticks_per_beat
        events = []        # (abs_tick, note, vel)
        seen_notes = set() # 디버그용: 어떤 노트가 실제로 들어왔는지

        for track in mid.tracks:
            cur = 0
            for msg in track:
                cur += msg.time
                if msg.type == "note_on" and msg.velocity > 0:
                    events.append((cur, msg.note, msg.velocity))
                    seen_notes.add(msg.note)

        events.sort(key=lambda x: x[0])

        grids = [{"kick":[0]*STEPS_PER_BAR, "snare":[0]*STEPS_PER_BAR} for _ in range(max_bars)]

        for abs_tick, note, vel in events:
            if vel < velocity_min:
                continue
            beat = abs_tick / tpq
            if beat < 0:
                continue
            bar_idx = int(beat // BAR_LEN_BEATS)
            if bar_idx >= max_bars:
                break
            pos  = beat - bar_idx * BAR_LEN_BEATS
            step = int(round((pos / BAR_LEN_BEATS) * STEPS_PER_BAR)) % STEPS_PER_BAR

            if note in KICK_NOTES:
                grids[bar_idx]["kick"][step] = 1
            elif note in SNARE_NOTES:
                grids[bar_idx]["snare"][step] = 1
            # (그 외 드럼은 무시)

        if verbose:
            print(f"[DEBUG] unique notes seen in MIDI: {sorted(seen_notes)}")
        return grids

    def _ensure_two_bars(g):
        if len(g) >= 2:
            return g[:2]
        if len(g) == 1:
            g0 = g[0]
            g1 = {"kick": g0["kick"][:], "snare": g0["snare"][:]}
            return [g0, g1]
        Z = {"kick":[0]*STEPS_PER_BAR, "snare":[0]*STEPS_PER_BAR}
        return [Z, Z]

    def _concat_2bars_to_vector(g2):
        """2마디 -> 64차원 벡터(kick32 + snare32)"""
        def flat(inst): return g2[0][inst] + g2[1][inst]
        return np.array(flat("kick") + flat("snare"), dtype=np.uint8)

    def _hamming_sim_ks(vecA: np.ndarray, vecB: np.ndarray, w=(0.6, 0.4)) -> float:
        """킥/스네어 구간별 해밍 유사도 가중 평균"""
        if vecA.shape != (64,) or vecB.shape != (64,):
            raise ValueError("유사도 계산은 64차원 벡터만 지원합니다. (kick32|snare32)")
        kW, sW = w
        kA, sA = vecA[0:32],  vecA[32:64]
        kB, sB = vecB[0:32],  vecB[32:64]
        sim = lambda a, b: float(np.sum(a == b)) / a.size
        kSim, sSim = sim(kA, kB), sim(sA, sB)
        return (kSim * kW + sSim * sW) / (kW + sW)

    # ---------- 입력/캐시 확인 ----------
    if not os.path.isfile(cache_path):
        raise FileNotFoundError(f"캐시 파일이 없습니다: {cache_path}")
    if not os.path.isfile(midi_path):
        raise FileNotFoundError(f"입력 MIDI를 찾을 수 없습니다: {midi_path}")

    npz  = np.load(cache_path, allow_pickle=True)
    names   = npz["names"]          # object array of str
    vectors = npz["vectors"]        # (N, 64)
    meta    = npz["meta"][0]        # dict

    if vectors.ndim != 2 or vectors.shape[1] != 64:
        raise ValueError("캐시 레이아웃이 킥/스네어 전용(64차원)과 다릅니다. 캐시를 다시 빌드하세요.")

    # ---------- 입력 MIDI -> 64차원 벡터 ----------
    g    = _midi_to_grids_01(midi_path,
                             velocity_min=meta.get("velocity_min", 1),
                             max_bars=meta.get("target_bars", TARGET_BARS))
    qvec = _concat_2bars_to_vector(_ensure_two_bars(g))

    # ---------- 라이브러리와 유사도 계산 → 최고 점수 ----------
    best_idx = -1
    best_sim = -1.0
    for i, v in enumerate(vectors):
        sim = _hamming_sim_ks(qvec, v, w=weights)
        if sim > best_sim:
            best_sim = sim
            best_idx = i

    best_name = str(names[best_idx])

    if verbose:
        _pretty_print_ks("입력 패턴 (64차원)", qvec)
        _pretty_print_ks(f"매칭된 패턴: {best_name} (score={best_sim:.4f})", vectors[best_idx])

    return (best_name, float(best_sim)) if return_score else best_name


# ───────────────────────────────
# 실행 구문
# ───────────────────────────────
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="2마디 킥/스네어 01 시퀀스 매칭 및 출력")
    parser.add_argument("--midi",  type=str, default="input_1.mid", help="비교할 MIDI 파일 경로")
    parser.add_argument("--cache", type=str, default="library_cache.npz", help="캐시(npz) 파일 경로")
    parser.add_argument("--kw",    type=float, default=0.6, help="킥 가중치 (default=0.6)")
    parser.add_argument("--sw",    type=float, default=0.4, help="스네어 가중치 (default=0.4)")
    parser.add_argument("--quiet", action="store_true", help="verbose 출력 끄기")
    parser.add_argument("--score", action="store_true", help="(파일명, 유사도) 튜플 반환 모드")
    args = parser.parse_args()

    result = match_best_from_cache(
        midi_path=args.midi,
        cache_path=args.cache,
        weights=(args.kw, args.sw),
        return_score=args.score,
        verbose=(not args.quiet)
    )

    print("\n>>> 최종 반환값:", result)
