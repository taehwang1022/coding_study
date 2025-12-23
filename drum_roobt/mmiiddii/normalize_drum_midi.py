
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Normalize (clean) an e-drum MIDI take for Magenta Drums RNN seeding.
- Tempo & grid normalization
- Flam/duplicate merge by IOI threshold
- Drum pitch canonicalization (kick/snare/hihat/toms/crash/ride)
- Velocity denoise + compression (+ optional bucketing)
- Oneshot-style fixed note_off length

Usage:
  python3 normalize_drum_midi.py --in raw_take.mid --out clean_seed.mid \
      --target-bpm 120 --grid 16 --ioi-merge-ms 30 --min-vel 12 --velocity-bucket

Requires: mido
"""

# cd /home/taehwang/basic-algo-lecture-master/drum_roobt/mmiiddii

# # 예시 실행
# python3 normalize_drum_midi.py --in raw_take.mid --out clean_seed.mid \
#   --target-bpm 120 --grid 16 --ioi-merge-ms 30 --min-vel 12 --velocity-bucket
import argparse
from collections import defaultdict
import math

import mido
from mido import MidiFile, MidiTrack, Message, MetaMessage, bpm2tempo, second2tick, tick2second

# Canonical drum mapping (GM-like)
CANON = {
    35: 36, 36: 36,                 # Kick
    37: 38, 38: 38, 39: 38, 40: 38, # Snare family -> 38
    42: 42, 44: 44, 46: 46,         # Hi-hat closed/foot/open
    41: 45, 43: 45, 45: 45,         # Low toms -> 45
    47: 47, 48: 47,                 # Mid toms -> 47
    50: 50, 52: 50,                 # High toms -> 50
    49: 49, 55: 49, 57: 49,         # Crashes -> 49
    51: 51, 53: 51, 59: 51          # Rides -> 51
}
KEEP = {36, 38, 42, 44, 46, 45, 47, 50, 49, 51}

def map_drum_pitch(p: int) -> int:
    p = CANON.get(p, p)
    if p in KEEP:
        return p
    # fallback heuristics
    if 35 <= p <= 36: return 36
    if 37 <= p <= 40: return 38
    if 41 <= p <= 50:
        # map to nearest tom
        return min([45,47,50], key=lambda t: abs(t - p))
    if p in (52, 54): return 50
    if p in (55, 56, 57): return 49
    if p in (51, 53, 59): return 51
    return 38

def quantize_tick(tick: int, grid: int) -> int:
    return int(round(tick / grid) * grid)

def velocity_process(v: int, bucket: bool) -> int:
    # root compression then clamp to [20,110]
    vv = int((math.sqrt(max(v,1)/127.0) * 90) + 20)
    if bucket:
        if vv <= 40: vv = 35
        elif vv >= 95: vv = 110
        else: vv = 75
    return max(1, min(127, vv))

def parse_args():
    ap = argparse.ArgumentParser(description="Normalize e-drum MIDI for Magenta")
    ap.add_argument("--in", dest="in_path", required=True, help="입력 MIDI 파일")
    ap.add_argument("--out", dest="out_path", required=True, help="출력 MIDI 파일")
    ap.add_argument("--target-bpm", type=int, default=120, help="정규화된 BPM")
    ap.add_argument("--grid", type=int, default=16, choices=[8,12,16,24,32], help="양자화 그리드(분모)")
    ap.add_argument("--ioi-merge-ms", type=float, default=30.0, help="같은 악기 연속타 merge 임계(ms)")
    ap.add_argument("--fixed-len-ms", type=float, default=90.0, help="드럼 note 길이(ms)")
    ap.add_argument("--min-vel", type=int, default=12, help="노이즈컷: 이 값 미만은 제거")
    ap.add_argument("--velocity-bucket", action="store_true", help="벨로시티 3단 버킷화 적용")
    ap.add_argument("--respect-hh-cc4", action="store_true", help="CC#4(하이햇 페달)로 열림/닫힘 추정")
    return ap.parse_args()

def main():
    args = parse_args()

    # Load source MIDI
    src = MidiFile(args.in_path)
    src_tpb = src.ticks_per_beat or 480

    # Read first tempo, fallback 120 BPM
    src_tempo = None
    for tr in src.tracks:
        for msg in tr:
            if msg.is_meta and msg.type == 'set_tempo':
                src_tempo = msg.tempo
                break
        if src_tempo is not None:
            break
    if src_tempo is None:
        src_tempo = bpm2tempo(120)

    # Collect note-on events (seconds), and CC4 if requested
    abs_tick = 0
    notes = []  # (abs_tick, note, vel, on/off)
    hh_cc = []  # (abs_tick, value)
    for msg in src:  # merged iteration across tracks
        abs_tick += msg.time
        if msg.type == 'note_on' and msg.velocity > 0:
            notes.append((abs_tick, msg.note, msg.velocity, 'on'))
        elif msg.type in ('note_off',) or (msg.type == 'note_on' and msg.velocity == 0):
            notes.append((abs_tick, msg.note, 0, 'off'))
        elif args.respect_hh_cc4 and msg.type == 'control_change' and msg.control == 4:
            hh_cc.append((abs_tick, msg.value))

    def tick_to_src_sec(t): return tick2second(t, src_tpb, src_tempo)

    # Build HH state function (open/closed) based on latest prior CC4 value
    def hh_state_at(tick: int) -> int:
        if not hh_cc:
            return 42  # default closed
        v = 42  # default closed note
        last_val = None
        for tt, val in hh_cc:
            if tt <= tick:
                last_val = val
            else:
                break
        if last_val is None:
            return 42
        if last_val >= 90: return 46  # open
        if last_val <= 30: return 42  # closed
        return 42

    # Keep only note_on for processing; apply pitch mapping and min_vel
    raw_hits = []  # (sec, pitch, vel)
    for t, p, v, kind in notes:
        if kind != 'on': 
            continue
        if v < args.min_vel:
            continue
        mp = map_drum_pitch(p)
        if mp in (42, 44, 46) and args.respect_hh_cc4:
            mp = hh_state_at(t)
        sec = tick_to_src_sec(t)
        raw_hits.append([sec, mp, v])

    # Sort & merge flams within IOI threshold per pitch
    raw_hits.sort(key=lambda x: x[0])
    merged = []
    last_time_by_pitch = {}
    for sec, p, v in raw_hits:
        last = last_time_by_pitch.get(p, -1e9)
        if (sec - last) * 1000.0 < args.ioi_merge_ms:
            # merge into previous same-pitch hit
            if merged and merged[-1][1] == p:
                merged[-1][2] = max(merged[-1][2], v)
                merged[-1][0] = min(merged[-1][0], sec)
            last_time_by_pitch[p] = sec
            continue
        merged.append([sec, p, v])
        last_time_by_pitch[p] = sec

    # Prepare destination MIDI with normalized tempo/TPB
    TPB = 480
    tempo_dst = bpm2tempo(args.target_bpm)
    out = MidiFile(ticks_per_beat=TPB)
    tr = MidiTrack()
    out.tracks.append(tr)
    tr.append(MetaMessage('set_tempo', tempo=tempo_dst, time=0))
    tr.append(MetaMessage('time_signature', numerator=4, denominator=4, time=0))

    def sec_to_dst_tick(sec: float) -> int:
        return int(second2tick(sec, TPB, tempo_dst))

    # Quantize to grid (e.g., 16th -> TPB/4)
    if args.grid == 8:
        grid = TPB // 2    # 8th
    elif args.grid == 12:
        grid = TPB // 3    # 12th (swing/triple)
    elif args.grid == 16:
        grid = TPB // 4    # 16th
    elif args.grid == 24:
        grid = TPB // 6    # 24th
    elif args.grid == 32:
        grid = TPB // 8    # 32nd
    else:
        grid = TPB // 4

    q_hits = []
    for sec, p, v in merged:
        tick = sec_to_dst_tick(sec)
        qtick = quantize_tick(tick, grid)
        q_hits.append([qtick, p, velocity_process(v, args.velocity_bucket)])

    # Collapse duplicates at same (tick,pitch) keeping highest velocity
    best = defaultdict(int)
    for t, p, v in q_hits:
        key = (t, p)
        if v > best[key]:
            best[key] = v

    events = sorted([(t, p, v) for (t, p), v in best.items()], key=lambda x: x[0])

    # Write as onshot notes with fixed length
    fixed_len_ticks = sec_to_dst_tick(args.fixed_len_ms / 1000.0)
    last_tick = 0
    for t, p, v in events:
        dt = max(0, t - last_tick)
        tr.append(Message('note_on', channel=9, note=p, velocity=v, time=dt))
        last_tick = t
        off_t = t + fixed_len_ticks
        tr.append(Message('note_off', channel=9, note=p, velocity=0, time=max(0, off_t - last_tick)))
        last_tick = off_t

    out.save(args.out_path)
    print(f"✅ Saved normalized MIDI → {args.out_path}")

if __name__ == "__main__":
    main()
