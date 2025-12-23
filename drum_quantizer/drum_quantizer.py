#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
drum-quantizer (STRICT 0.125s grid @ 120 BPM)
- 전 구간을 0.125초 그리드(= 16th @ 120BPM)로 고정
- 시작은 가까운 0.125s 경계, 끝은 다음 0.125s 경계로 스냅
- 같은 (pitch, grid_idx) 겹치면 velocity 최댓값 1개만 남김
- 드럼 트랙만 처리, 비드럼은 그대로
- 출력: 입력파일명 + "_양자화.mid"
"""

import os
from collections import defaultdict
from typing import Dict, List, Tuple
import pretty_midi
import math

GRID_SEC = 0.125          # 120 BPM 기준 16분음표 길이
MIN_DUR_SEC = 0.001       # 최소 길이 1ms
EPS = 1e-9

def to_grid_idx(t: float) -> int:
    # 가까운 0.125s 배수의 인덱스 (반올림). 음수 방지
    return max(0, int(round(t / GRID_SEC)))

def grid_time(idx: int) -> float:
    return idx * GRID_SEC

def quantize_drums_fixed_grid_inplace(pm: pretty_midi.PrettyMIDI) -> None:
    """
    드럼 노트:
      - start -> 최근접 GRID_SEC 배수
      - end   -> 다음 GRID_SEC 경계(최소 1ms)
      - (pitch, grid_idx) 중복은 velocity 최댓값 1개
    """
    song_end = pm.get_end_time()

    # (pitch, grid_idx) -> 후보 리스트
    bucket: Dict[Tuple[int, int], List[pretty_midi.Note]] = defaultdict(list)

    for inst in pm.instruments:
        if not inst.is_drum:
            continue
        for note in inst.notes:
            # 시작 그리드 스냅
            idx = to_grid_idx(max(0.0, note.start))
            snapped_start = grid_time(idx)

            # 끝을 다음 경계로 (마지막을 넘어가면 곡 끝으로)
            next_boundary = grid_time(idx + 1)
            snapped_end = max(snapped_start + MIN_DUR_SEC,
                              min(next_boundary, song_end))

            # 노트 생성
            bucket[(note.pitch, idx)].append(pretty_midi.Note(
                velocity=note.velocity,
                pitch=note.pitch,
                start=snapped_start,
                end=snapped_end
            ))

    # 중복 해소: 각 (pitch, idx)에서 velocity 최댓값 1개
    picked: List[pretty_midi.Note] = []
    for (pitch, idx), cands in bucket.items():
        best = max(cands, key=lambda n: n.velocity)
        picked.append(best)

    # 드럼 트랙 재구성(단일 트랙)
    non_drums = [i for i in pm.instruments if not i.is_drum]
    drum_out = pretty_midi.Instrument(program=0, is_drum=True, name="Drums(quantized-0.125s)")
    drum_out.notes = sorted(picked, key=lambda n: n.start)
    pm.instruments = non_drums + [drum_out]

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

    quantize_drums_fixed_grid_inplace(pm)

    base, _ = os.path.splitext(input_midi)
    output_midi = f"{base}_quantizer.mid"
    try:
        pm.write(output_midi)
    except Exception as e:
        print(f"❌ 저장 실패: {e}")
        return

    print(f"✅ 저장 완료: {output_midi}")

if __name__ == "__main__":
    main()
