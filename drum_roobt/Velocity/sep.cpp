#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

static inline bool isDrum(int inst) {
    // 1~4: 드럼, 10: 베이스(드럼)
    return (inst >= 1 && inst <= 4) || inst == 10;
}
static inline bool isCymbal(int inst) {
    // 5~8: 심벌, 11: 하이햇(심벌)
    return (inst >= 5 && inst <= 8) || inst == 11;
}

int main() {
    std::ios::sync_with_stdio(false);

    // 입력 파일 경로
    std::string inPath;
    std::cout << "입력 파일명: ";
    std::cin >> inPath;

    // 출력 파일명: 확장자 앞에 _cd
    std::string outPath;
    size_t dot = inPath.find_last_of('.');
    if (dot != std::string::npos) outPath = inPath.substr(0, dot) + "_cd" + inPath.substr(dot);
    else outPath = inPath + "_cd.txt";

    std::ifstream in(inPath);
    if (!in.is_open()) {
        std::cerr << "[ERROR] 입력 파일 열기 실패: " << inPath << "\n";
        return 1;
    }
    std::ofstream out(outPath);
    if (!out.is_open()) {
        std::cerr << "[ERROR] 출력 파일 열기 실패: " << outPath << "\n";
        return 2;
    }

    // 헤더
    out << "time\tcymbal_vel\tdrum_vel\n";

    std::string line;
    double t; int inst, vel;

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        // 콤마 → 공백 변환(혼용 지원)
        for (char &c : line) if (c == ',') c = ' ';

        std::stringstream ss(line);
        if (!(ss >> t >> inst >> vel)) continue; // 형식 불량 스킵
        if (t == -1) break;                      // 종료 신호

        int cym = 0, drum = 0;
        if (isDrum(inst))       drum = vel;      // 1~4, 10
        else if (isCymbal(inst)) cym  = vel;     // 5~8, 11
        // 그 외/0은 둘 다 0

        out << std::fixed << std::setprecision(3)
            << t << "\t" << cym << "\t" << drum << "\n";
    }

    std::cout << "[완료] 파싱 저장: " << outPath << "\n";
    return 0;
}
