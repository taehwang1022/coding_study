#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>

std::vector<std::string> splitByWhitespace(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string temp;
    while (iss >> temp) {
        tokens.push_back(temp);
    }
    return tokens;
}

bool compareFilesIgnoringHihat(const std::string& file1, const std::string& file2) {
    std::ifstream input1(file1);
    std::ifstream input2(file2);

    if (!input1.is_open()) {
        std::cerr << "파일 열기 실패: " << file1 << "\n";
        return false;
    }
    if (!input2.is_open()) {
        std::cerr << "파일 열기 실패: " << file2 << "\n";
        return false;
    }

    std::string line1, line2;
    int lineNum = 1;
    int diffCount = 0;
    bool isSame = true;

    while (std::getline(input1, line1) && std::getline(input2, line2)) {
        auto tokens1 = splitByWhitespace(line1);
        auto tokens2 = splitByWhitespace(line2);

        if (tokens1.size() < 6 || tokens2.size() < 6) {
            std::cerr << "줄 " << lineNum << "에서 필드 수 부족\n";
            diffCount++;
            lineNum++;
            continue;
        }

        bool mismatch = false;
        for (int i = 0; i < 6; ++i) {
            if (tokens1[i] != tokens2[i]) {
                mismatch = true;
                break;
            }
        }

        if (mismatch) {
            std::cout << "차이 발견 (줄 " << lineNum << ")\n";
            std::cout << "파일1: ";
            for (const auto& token : tokens1)
                std::cout << std::setw(8) << token;
            std::cout << "\n파일2: ";
            for (const auto& token : tokens2)
                std::cout << std::setw(8) << token;
            std::cout << "\n\n";
            isSame = false;
            diffCount++;
        }
        
        lineNum++;
    }

    while (std::getline(input1, line1)) {
        std::cout << "파일1 추가 줄 (줄 " << lineNum << "): [" << line1 << "]\n";
        isSame = false;
        diffCount++;
        lineNum++;
    }
    while (std::getline(input2, line2)) {
        std::cout << "파일2 추가 줄 (줄 " << lineNum << "): [" << line2 << "]\n";
        isSame = false;
        diffCount++;
        lineNum++;
    }

    std::cout << "총 서로 다른 줄 수: " << diffCount << "줄\n";

    return isSame;
}

int main() {
    std::string file1, file2;
    std::cout << "첫 번째 파일 경로를 입력하세요: ";
    std::cin >> file1;
    std::cout << "두 번째 파일 경로를 입력하세요: ";
    std::cin >> file2;

    bool result = compareFilesIgnoringHihat(file1, file2);
    if (result) {
        std::cout << "두 파일은 완전히 동일합니다 (하이햇 제외).\n";
    } else {
        std::cout << "두 파일에 차이가 있습니다 (하이햇 제외).\n";
    }

    return 0;
}