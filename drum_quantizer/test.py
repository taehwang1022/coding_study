#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
drum-quantizer + hit-time printer (STRICT 0.125s grid @ 120 BPM)
- 전 곡을 0.125초 그리드(= 16th @ 120BPM)로 고정
- 시작은 가까운 0.125s 경계, 끝은 다음 0.125s 경계로 스냅
- 같은 (pitch, grid_idx) 겹치면 velocity 최댓값 1개만 남김
- 드럼 트랙만 처리(비드럼 보존)
- 콘솔에 원본/스냅 타격시간을 프린트
- 저장 파일명: 입력 + "_양자화.mid"
"""

import os
from collections import defaultdict
from typing import Dict, List, Tuple
import pretty_midi

GRID_SEC = 0.125        # 120 BPM 16분음표 길이(초)
MIN_DUR_SEC = 0.001     # 최소 길이 1ms

def to_grid_idx(t: float) -> int:
    return max(0, int(round(t / GRID_SEC)))

def grid_time(idx: int) -> float:
    return idx * GRID_SEC

def quantize_and_collect_times(pm: pretty_midi.PrettyMIDI):
    """
    드럼 노트 양자화(0.125s 격자) + (원본, 스냅) 타격 시각 수집
    반환:
        raw_hits   : [(start_sec, pitch, vel), ...]  (원본)
        snapped    : [pretty_midi.Note, ...]         (중복 정리 후 스냅된 노트)
    """
    song_end = pm.get_end_time()
    raw_hits: List[Tuple[float, int, int]] = []  # (start, pitch, velocity)

    # (pitch, grid_idx) -> 후보 노트들
    bucket: Dict[Tuple[int, int], List[pretty_midi.Note]] = defaultdict(list)

    for inst in pm.instruments:
        if not inst.is_drum:
            continue
        for note in inst.notes:
            raw_hits.append((note.start, note.pitch, note.velocity))

            idx = to_grid_idx(max(0.0, note.start))
            snapped_start = grid_time(idx)
            next_boundary = grid_time(idx + 1)
            snapped_end = max(snapped_start + MIN_DUR_SEC, min(next_boundary, song_end))

            bucket[(note.pitch, idx)].append(pretty_midi.Note(
                velocity=note.velocity,
                pitch=note.pitch,
                start=snapped_start,
                end=snapped_end
            ))

    # 중복 해소: 각 (pitch, idx)에서 velocity 최댓값 1개만 남김
    picked: List[pretty_midi.Note] = []
    for (pitch, idx), cands in bucket.items():
        best = max(cands, key=lambda n: n.velocity)
        picked.append(best)

    return raw_hits, sorted(picked, key=lambda n: n.start)

def rebuild_drums(pm: pretty_midi.PrettyMIDI, snapped_notes: List[pretty_midi.Note]):
    """드럼 트랙을 하나로 재구성하고 스냅된 노트로 대체."""
    non_drums = [i for i in pm.instruments if not i.is_drum]
    drum_out = pretty_midi.Instrument(program=0, is_drum=True, name="Drums(quantized-0.125s)")
    drum_out.notes = snapped_notes
    pm.instruments = non_drums + [drum_out]

def print_hit_times(raw_hits: List[Tuple[float, int, int]], snapped_notes: List[pretty_midi.Note]):
    """콘솔에 원본/스냅 타격 시간 출력."""
    # 원본: 시간순 정렬
    raw_sorted = sorted(raw_hits, key=lambda x: x[0])
    print("\n[원본 타격 시각(초) | pitch | vel]")
    for t, p, v in raw_sorted:
        print(f"{t:.6f}\t{p}\t{v}")

    print("\n[양자화(0.125s 그리드) 타격 시각(초) | idx | pitch | vel]")
    for n in snapped_notes:
        idx = to_grid_idx(n.start)
        print(f"{n.start:.3f}\t{idx}\t{n.pitch}\t{n.velocity}")

def main():
    input_midi = input("🎵 입력 MIDI 파일 이름을 입력하세요: ").strip()
    if not os.path.isfile(input_midi):
        print(f"❌ 파일을 찾을 수 없습니다: {input_midi}")
        return

    try:
        pm = pretty_midi.PrettyMIDI(input_midi)
    except Exception as e:
        print(f"❌ MIDI 로드 실패: {e}")
        return

    # 양자화 + 타격 시각 수집
    raw_hits, snapped_notes = quantize_and_collect_times(pm)

    # 출력(프린트)
    print_hit_times(raw_hits, snapped_notes)

    # 재구성 및 저장
    rebuild_drums(pm, snapped_notes)
    base, _ = os.path.splitext(input_midi)
    output_midi = f"{base}_양자화.mid"
    try:
        pm.write(output_midi)
    except Exception as e:
        print(f"❌ 저장 실패: {e}")
        return

    print(f"\n✅ 저장 완료: {output_midi}")

if __name__ == "__main__":
    main()
