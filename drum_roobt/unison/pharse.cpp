#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

// 함수 선언
void processFiles(const std::string &inputFilePath, const std::string &outputFilePath);

int main()
{
    // 입력 파일 경로와 출력 파일 경로 설정
    std::string inputFilePath = "/home/taehwang/basic-algo-lecture-master/drum_roobt/unison/log_unison.txt";
    std::string outputFilePath = "/home/taehwang/basic-algo-lecture-master/drum_roobt/unison/output.txt";

    std::cout << "Hello World!" << std::endl;

    // 함수 호출
    processFiles(inputFilePath, outputFilePath);

    return 0;
}

// 함수 정의
void processFiles(const std::string &inputFilePath, const std::string &outputFilePath)
{
    std::ifstream inFile(inputFilePath);
    std::ofstream outFile(outputFilePath);

    if (!inFile.is_open() || !outFile.is_open())
    {
        std::cerr << "파일을 열 수 없습니다." << std::endl;
        return;
    }

    std::string line;
    std::vector<std::string> buffer;
    std::vector<double> times;

    while (std::getline(inFile, line))
    {
        buffer.push_back(line);

        // 시간 추출
        std::istringstream iss(line);
        double time;
        iss >> time;
        times.push_back(time);

        // 4줄씩 처리
        if (buffer.size() == 4)
        {
            double sum = 0.0;
            for (double t : times)
                sum += t;
            double bpm = 60.0 / (sum / 4.0);

            // bpm 정보 삽입
            outFile << "bpm: " << bpm << std::endl;

            // 4줄 출력
            for (const auto &bufLine : buffer)
            {
                outFile << bufLine << std::endl;
            }

            buffer.clear();
            times.clear();
        }
    }

    // 남은 줄 처리 (4줄 미만)
    if (!buffer.empty())
    {
        double sum = 0.0;
        for (double t : times)
            sum += t;
        double bpm = sum / 2.4 * 100;

        // bpm 정보 삽입
        outFile << "bpm: " << bpm << std::endl;

        // 남은 줄 출력
        for (const auto &bufLine : buffer)
        {
            outFile << bufLine << std::endl;
        }
    }
    inFile.close();
    outFile.close();
}