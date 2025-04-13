#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>

struct DrumEvent {
    double time;
    int rightInstrument;
    int leftInstrument;
    int rightPower;
    int leftPower;
    int isBass;
    int hihatOpen;
};

int main() {
    std::string fileBaseName;
    std::cout << "입력 파일 이름을 입력하세요 : ";
    std::cin >> fileBaseName;

    std::string inputDir  = "/home/taehwang/basic-algo-lecture-master/drum_roobt/midifile/chord_assign/";
    std::string outputDir = "/home/taehwang/basic-algo-lecture-master/drum_roobt/midifile/chord_box/";

    std::string inputFilename  = inputDir + fileBaseName + "_a.csv";
    std::string outputFilename = outputDir + fileBaseName + "_final.txt";

    std::ifstream input(inputFilename);
    if (!input.is_open())
    {
        std::cerr << "입력 파일 열기 실패: " << inputFilename << "\n";
        return 1;
    }

    std::ofstream output(outputFilename);
    if (!output.is_open())
    {
        std::cerr << "출력 파일 생성 실패: " << outputFilename << "\n";
        return 1;
    }

    std::string line;
    std::vector<DrumEvent> result;

    while (std::getline(input, line)) {
        std::stringstream ss(line);
        DrumEvent ev;
        ss >> ev.time >> ev.rightInstrument >> ev.leftInstrument
           >> ev.rightPower >> ev.leftPower >> ev.isBass >> ev.hihatOpen;

        double remaining = ev.time;

        // 쪼개면서 처리
        while (remaining > 0.6) {
            DrumEvent mid;
            mid.time = 0.6;
            mid.rightInstrument = 0;
            mid.leftInstrument  = 0;
            mid.rightPower      = 0;
            mid.leftPower       = 0;
            mid.isBass          = 0;
            mid.hihatOpen       = 0;
            result.push_back(mid);
            remaining -= 0.6;
        }

        // 마지막 남은 시간 (0.6 이하)
        if (remaining > 0.0) { 
            DrumEvent last = ev;
            last.time = remaining;
            result.push_back(last);
        }
    }

    // 맨 앞에 dummy 한 줄 추가
    output << "1\t 0.600\t 0\t 0\t 0\t 0\t 0\t 0\n";
    // 마디 계산 (2.4초마다 증가)
    double measureTime = 0.0;
    int measureNum = 1;
    const double EPS = 1e-6;
    const double MEASURE_LIMIT = 2.4;

    for (const auto& ev : result) {
        measureTime += ev.time;
        if (measureTime + EPS >= MEASURE_LIMIT) {
            measureNum++;
            measureTime = 0.0;
        }
        output << measureNum << "\t "
             << std::fixed << std::setprecision(3) << ev.time << "\t "
             << ev.rightInstrument << "\t "
             << ev.leftInstrument << "\t "
             << ev.rightPower << "\t "
             << ev.leftPower << "\t "
             << ev.isBass << "\t "
             << ev.hihatOpen << "\n";
    
    }

    output << measureNum+1 << "\t 0.600\t 0\t 0\t 0\t 0\t 0\t 0\n";
    output << measureNum+1 << "\t 0.600\t 1\t 1\t 1\t 1\t 1\t 1\n";
    

    std::cout << "코드 끗.";
    return 0;
}
