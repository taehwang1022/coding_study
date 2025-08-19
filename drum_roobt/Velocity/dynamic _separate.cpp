#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <string>
#include <sstream>

struct VelocityEntry {
    double time;
    int instrument;
    int velocity;
};

void analyzeVelocityWithLowPassFilter(const std::string& velocityFile,
                                      const std::string& outputFile,
                                      double windowSize = 0.6)
{
    std::ifstream in(velocityFile);
    if (!in.is_open()) {
        std::cerr << "velocityFile 열기 실패: " << velocityFile << "\n";
        return;
    }

    std::vector<VelocityEntry> rawData;
    double t;
    int inst, vel;
    double maxTime = 0.0;

    // CSV 파일 읽기
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
    
        // 콤마 → 공백 변환 (콤마/공백 혼합 지원)
        for (char &c : line) {
            if (c == ',') c = ' ';
        }
    
        std::stringstream ss(line);
        if (!(ss >> t >> inst >> vel)) continue; // 읽기 실패 시 건너뜀


        if (t == -1) break; // 종료 신호

    
        rawData.push_back({t, inst, vel});
        if (t > maxTime) maxTime = t;
    }

    int numWindows = static_cast<int>(std::ceil((maxTime + 0.001) / windowSize));

    std::vector<double> avgVelocityDrum(numWindows, 0.0);  // 1~4
    std::vector<int> countDrum(numWindows, 0);
    
    std::vector<double> avgVelocityCym(numWindows, 0.0);   // 5~8
    std::vector<int> countCym(numWindows, 0);

    // 시간 구간별 누적
    for (const auto& entry : rawData) {
        int bin = static_cast<int>(entry.time / windowSize);
        if (bin >= numWindows) continue;

        if (entry.instrument >= 1 && entry.instrument <= 4) {
            avgVelocityDrum[bin] += entry.velocity;
            countDrum[bin]++;
        }
        else if (entry.instrument >= 5 && entry.instrument <= 8) {
            avgVelocityCym[bin] += entry.velocity;
            countCym[bin]++;
        }
    }

    for (int i = 0; i < numWindows; ++i) {
        if (countDrum[i] > 0) {
            avgVelocityDrum[i] = static_cast<int>(std::round((avgVelocityDrum[i] / countDrum[i]) / 40.0));
        }
        if (countCym[i] > 0) {
            avgVelocityCym[i] = static_cast<int>(std::round((avgVelocityCym[i] / countCym[i]) / 40.0));
        }
    }
    
    // 결과 저장
    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "outputFile 열기 실패: " << outputFile << "\n";
        return;
    }

    out << "start_time\tend_time\tdrum_avg\tcymbal_avg\n";
    for (int i = 0; i < numWindows; ++i) {
        double startT = i * windowSize;
        double endT = (i + 1) * windowSize;
        out << std::fixed << std::setprecision(3)
            << startT << "\t" << endT << "\t"
            << avgVelocityDrum[i] << "\t" << avgVelocityCym[i] << "\n";
    }

    std::cout << "[완료] 드럼/심벌 평균 벨로시티 저장 완료: " << outputFile << "\n";
}

int main()
{
    std::string velocityFile;
    std::cout << "입력 파일명: ";
    std::cin >> velocityFile;
    
    std::string outputFile;
    size_t dotPos = velocityFile.find_last_of('.');
    if (dotPos != std::string::npos) {
        outputFile = velocityFile.substr(0, dotPos) + "_s" + velocityFile.substr(dotPos);
    } else {
        outputFile = velocityFile + "_s.txt";
    }

    double windowSize = 0.6;  // 1초 단위로 구간 나누기

    analyzeVelocityWithLowPassFilter(velocityFile, outputFile, windowSize);
    
    return 0;
}
