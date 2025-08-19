#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <string>

struct VelocityEntry {
    double time;
    int instrument;
    int velocity;
};

void analyzeVelocityWithLowPassFilter(const std::string& velocityFile,
                                      const std::string& outputFile,
                                      //나중에 윈도우 사이즈는 bpm 따라 바꿔주기
                                      double windowSize = 1.0)
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

    // 벨로시티 파일 읽기
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
    
        std::stringstream ss(line);
        std::string token;
    
        std::getline(ss, token, ','); t = std::stod(token);
        std::getline(ss, token, ','); inst = std::stoi(token);
        std::getline(ss, token, ','); vel = std::stoi(token);
    
        rawData.push_back({t, inst, vel});
        if (t > maxTime) maxTime = t;
    }
    

    int numWindows = static_cast<int>(std::ceil((maxTime + 0.001) / windowSize));
    std::vector<double> avgVelocity(numWindows, 0.0);
    std::vector<int> counts(numWindows, 0);

    // 시간 구간별 평균 계산
    for (const auto& entry : rawData) {
        int bin = static_cast<int>(entry.time / windowSize);
        avgVelocity[bin] += entry.velocity;
        counts[bin]++;
    }

    for (int i = 0; i < numWindows; ++i) {
        if (counts[i] > 0)
            avgVelocity[i] /= counts[i];
    }

    // 결과 저장
    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "outputFile 열기 실패: " << outputFile << "\n";
        return;
    }

    out << "start_time\tend_time\tavg_velocity\n";
    out << std::fixed << std::setprecision(3);
    for (int i = 0; i < numWindows; ++i) {
        double startT = i * windowSize;
        double endT = (i + 1) * windowSize;
        out << startT << "\t" << endT << "\t" << avgVelocity[i] << "\n";
    }

    std::cout << "[완료] 구간 평균 벨로시티 저장 완료: " << outputFile << "\n";
}


int main()
{
    std::string velocityFile = "test.csv";          // 입력: 시간 악기 세기 형식
    std::string outputFile   = "test-1.txt";   // 출력 파일

    double windowSize = 1.0;  // 1초 단위로 구간 나누기

    analyzeVelocityWithLowPassFilter(velocityFile, outputFile, windowSize);

    return 0;
}
