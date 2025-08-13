import matplotlib.pyplot as plt
import numpy as np
# python3 drum_roobt/Velocity/view.py
# 입력 받기
mode = input("📊 그래프 모드를 선택하세요 (1: 선그래프, 2: 막대그래프): ").strip()

# 파일 경로
filename = '/home/taehwang/basic-algo-lecture-master/drum_roobt/Velocity/test_s.txt'

# 데이터 리스트
start_time = []
drum_avg = []
cymbal_avg = []

# 파일 읽기
with open(filename, 'r') as file:
    next(file)  # 헤더 건너뜀
    for line in file:
        parts = line.strip().split('\t')
        if len(parts) == 4:
            start_time.append(float(parts[0]))
            drum_avg.append(float(parts[2]))
            cymbal_avg.append(float(parts[3]))

# 그래프 그리기
plt.figure(figsize=(12, 6))

if mode == '1':
    # 선 그래프
    plt.plot(start_time, drum_avg, marker='o', label='Drum Avg')
    plt.plot(start_time, cymbal_avg, marker='x', label='Cymbal Avg')
    plt.title('Drum & Cymbal Velocity over Time (Line)')
elif mode == '2':
    # 막대 그래프
    bar_width = 0.4
    x = np.array(start_time)
    x_drum = x - bar_width / 2
    x_cymbal = x + bar_width / 2

    plt.bar(x_drum, drum_avg, width=bar_width, label='Drum Avg')
    plt.bar(x_cymbal, cymbal_avg, width=bar_width, label='Cymbal Avg')
    plt.title('Drum & Cymbal Velocity over Time (Bar)')
else:
    print("잘못된 입력입니다. 1 또는 2를 입력하세요.")
    exit()

# 공통 설정
plt.xlabel('Time (s)')
plt.ylabel('Velocity')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
