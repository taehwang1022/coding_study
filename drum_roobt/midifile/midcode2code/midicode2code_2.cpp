#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>

struct Event {
    double time;
    std::vector<int> notes;
};

std::vector<std::string> splitByWhitespace(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string temp;
    while (iss >> temp) {
        tokens.push_back(temp);
    }
    return tokens;
}

int main() {
    std::string fileBaseName;
    int hihatState = 1; // 하이햇 초기 상태 (닫힘)

    std::cout << "입력 파일 이름을 입력 :  ";
    std::cin >> fileBaseName;

    std::string inputDir  = "/home/taehwang/basic-algo-lecture-master/drum_roobt/midifile/mid2midcode/";
    std::string outputDir = "/home/taehwang/basic-algo-lecture-master/drum_roobt/midifile/RLassign/";

    std::string inputFilename  = inputDir + fileBaseName + "_mc.csv";
    std::string outputFilename = outputDir + fileBaseName + "_c.csv";

    std::ifstream input(inputFilename);
    if (!input.is_open()) {
        std::cerr << " 입력 파일 열기 실패: " << inputFilename << "\n";
        return 1;
    }

    std::ofstream output(outputFilename);
    if (!output.is_open()) {
        std::cerr << " 출력 파일 생성 실패: " << outputFilename << "\n";
        return 1;
    }

    std::vector<Event> mergedEvents;
    std::string line;
    double currentTime = 0.0;

    while (std::getline(input, line)) {
        auto tokens = splitByWhitespace(line);
        if (tokens.size() != 2) {
            std::cerr << "파싱 실패 또는 필드 부족: [" << line << "]\n";
            continue;
        }

        try {
            double delta = std::stod(tokens[0]);
            int rawNote = std::stoi(tokens[1]);
            int mapped = rawNote;

            if (mapped < 1 || mapped > 11) {
                std::cerr << "허용되지 않은 노트 번호: " << mapped << " (줄: [" << line << "])\n";
                continue;
            }

            if (mergedEvents.empty() || delta > 0) {
                currentTime += delta;
                mergedEvents.push_back({currentTime, {mapped}});
            } else {
                mergedEvents.back().notes.push_back(mapped);
            }
        } catch (...) {
            std::cerr << "숫자 변환 실패: [" << line << "]\n";
            continue;
        }
    }

    double prevTime = 0.0;

    for (const auto& e : mergedEvents) {
        int inst1 = 0, inst2 = 0;
        int bassHit = 0;
        int hihat = hihatState;
    
        for (int note : e.notes) {
            if (note >= 1 && note <= 8) {
                if (inst1 == 0) inst1 = note;
                else if (inst2 == 0) inst2 = note;
            } else if (note == 10) {
                bassHit = 1;
            } else if (note == 11) {
                hihatState = 0; // Open
            } else if (note == 5) {
                hihatState = 1; // Closed
            }
        }
    
        hihat = hihatState;
        double deltaTime = e.time - prevTime;
        prevTime = e.time;
    
        output << std::fixed << std::setprecision(3)
               << std::setw(6) << deltaTime
               << std::setw(6) << inst1
               << std::setw(6) << inst2
               << std::setw(6) << 0
               << std::setw(6) << 0
               << std::setw(6) << bassHit
               << std::setw(6) << hihat << "\n";
    }
    
    std::cout << "변환 완료! 저장 위치 → " << outputFilename << "\n";
    return 0;
}
