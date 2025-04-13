#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>

struct Event {
    double time;
    int inst1 = 0;
    int inst2 = 0;
    int bassHit = 0;
    int hihat = 1;
    int rightHand = 0;
    int leftHand = 0;
};

std::vector<std::string> splitByWhitespace(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string temp;
    while (iss >> temp)
    {
        tokens.push_back(temp);
    }
    return tokens;
}

enum Hand {
    LEFT,
    RIGHT,
    SAME
};

struct Coord {
    double x;
    double y;
    double z;
};

Coord instPos[9] = {
    {0.0, 0.0, 0.0},         // 0번 없음
    {-0.13, 0.52, 0.61},      // 1: 스네어
    {0.25, 0.50, 0.62},      // 2: 하이햇
    {0.21, 0.67, 0.87},      // 3: 하이햇 오픈
    {-0.05, 0.69, 0.83},      // 4: 탑탐
    {-0.28,  0.60, 0.88},      // 5: 미드탐
    {0.32,  0.71, 1.06},      // 6: 라이드
    {0.47,  0.52, 0.88},      // 7: 크래시 오른쪽
    {-0.06, 0.73, 1.06}      // 8: 크래시 왼쪽
};

double dist(Coord a, Coord b) {
    return std::sqrt(
        (a.x - b.x)*(a.x - b.x) +
        (a.y - b.y)*(a.y - b.y) +
        (a.z - b.z)*(a.z - b.z)
    );
}



// ////////////////////////////거리 계산 로직
Hand getPreferredHandByDistance(int instCurrent, int prevRightNote, int prevLeftNote, double prevRightHit, double prevLeftHit) {
    if (instCurrent <= 0 || instCurrent >= 9) return RIGHT;

    Coord curr = instPos[instCurrent];
    Coord right = instPos[prevRightNote];
    Coord left = instPos[prevLeftNote];

    double dMax = 0.754; //dist(instPos[2], instPos[7]);
    double dRight = dist(curr, right);
    double dLeft = dist(curr, left);
    double tRight = prevRightHit;
    double tLeft = prevLeftHit;

    double real_tRight = tRight * (138 / 100.0);
    double real_tLeft  = tLeft  * (138 / 100.0);
    double normRight = std::min(dRight / dMax, 1.0);
    double normLeft = std::min(dLeft / dMax, 1.0);


    double rScore = (real_tRight/0.6) * (1-normRight);
    double lScore = (real_tLeft/0.6) * (1-normLeft);

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "\n[손 선택 판단]\n"
              << "  ▶ 예전 악기 오른손: " << prevRightNote << " | 왼손: " << prevLeftNote << "\n"
              << "  ▶ 대상 악기 inst: " << instCurrent << "\n"
              << "  ▶ 거리  - 오른손: " << dRight << " | 왼손: " << dLeft << " (최대 거리: " << dMax << ")\n"
              << "  ▶ 시간  - 오른손: " << prevRightHit << " 실제: " << real_tRight << " | 왼손: " << prevLeftHit << " 실제: " << real_tLeft << "\n"
              << "  ▶ 점수  - 오른손: " << rScore << " | 왼손: " << lScore << "\n";
              
    if (std::abs(rScore - lScore) < 1e-6) {
    std::cout << "  => 선택된 손: 동일 점수 → SAME\n";
    return SAME;
    }
    std::cout << "  => 선택된 손: " << ((lScore <= rScore) ? "오른손 (RIGHT)" : "왼손 (LEFT)") << "\n";

    return (lScore <= rScore) ? RIGHT : LEFT;
}

// /////////////////////여기까지

int main() {
    std::string fileBaseName;
    std::cout << "입력 파일 이름을 입력하세요 : ";
    std::cin >> fileBaseName;

    std::string inputDir  = "/home/taehwang/basic-algo-lecture-master/drum_roobt/midifile/RLassign/";
    std::string outputDir = "/home/taehwang/basic-algo-lecture-master/drum_roobt/midifile/chord_assign/";

    std::string inputFilename  = inputDir + fileBaseName + "_c.csv";
    std::string outputFilename = outputDir + fileBaseName + "_a.csv";

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
    std::vector<Event> events;
    int prevRight = 0, prevLeft = 0;
    int num =0;
    int changedCount = 0;
    int prevRightNote =1;
    int prevLeftNote =1;
    double prevRightHit =0;
    double prevLeftHit =0;

    while (std::getline(input, line))
    {
        auto tokens = splitByWhitespace(line);
        if (tokens.size() != 7)
        {
            std::cerr << "필드 수 부족: [" << line << "]\n";
            continue;
        }

        Event event;
        event.time   = std::stod(tokens[0]);
        event.inst1  = std::stoi(tokens[1]);
        event.inst2  = std::stoi(tokens[2]);
        event.bassHit = std::stoi(tokens[5]);
        event.hihat   = std::stoi(tokens[6]);

        int inst1 = event.inst1;
        int inst2 = event.inst2;
        int originalRight = prevRight;
        int originalLeft  = prevLeft;
        
        prevLeftHit += event.time;
        prevRightHit += event.time;

        // 손 어사인 로직

        // 항상 7번은 오른손으로
        if (inst1 == 7 || inst2 == 7)
        {
            if (inst1 == 7) {
                event.rightHand = 7;
                if (inst2 != 0) event.leftHand = inst2;
            } else {
                event.rightHand = inst2;
                if (inst1 != 0) event.leftHand = inst1;
            }
        }

        if (inst1 != 0 && inst2 != 0)
        {
            if (inst1 == 1 || inst2 == 1)
            {
                if (inst1 == 1)
                {
                    event.leftHand = inst1;
                    event.rightHand = inst2;
                }
                else
                {
                    event.leftHand = inst2;
                    event.rightHand = inst1;
                }
            }
            else
            {
                if (inst1 == 2 || inst1 == 3 || inst1 == 6 || inst1 == 7)
                {
                    event.rightHand = inst1;
                    event.leftHand  = inst2;
                }
                else
                {
                    event.rightHand = inst2;
                    event.leftHand  = inst1;
                }
            }
        }
        else if (inst1 != 0)
        {
            if (inst1 == prevRight || inst1 == prevLeft)
            {
                if (event.time <= 0.1)
                {
                    if (inst1 == prevRight)
                        event.rightHand = inst1;
                    else
                        event.leftHand = inst1;
                }
                else
                {
                    // /////////////////거리계산
                    Hand hand = getPreferredHandByDistance(inst1, prevRightNote, prevLeftNote, prevRightHit, prevLeftHit);
                    if (hand == RIGHT)
                        event.rightHand = inst1;
                    else if(hand == LEFT)
                        event.leftHand = inst1;
                    else { 
                        if (inst1 == 2 || inst1 == 3 || inst1 == 6 || inst1 == 7)
                            event.rightHand = inst1;
                        else
                            event.leftHand = inst1;
                    }
                    // /////////////////////여기까지
                }
            }
            else
            {
                if (inst1 == 2 || inst1 == 3 || inst1 == 6 || inst1 == 7)
                {
                    event.rightHand  = inst1;
                }
                else
                {
                    event.leftHand  = inst1;
                }
            }
        }

        if (event.rightHand != originalRight || event.leftHand != originalLeft)
        {
            std::cout << num << "//손 변경 감지 → 시간: " << event.time
                      << ", inst1: " << inst1 << ", inst2: " << inst2
                      << " | 오른손: " << event.rightHand << ", 왼손: " << event.leftHand << " TimeDiff : " << event.time << "\n";
            changedCount++;
        }

        prevRight = event.rightHand;
        prevLeft  = event.leftHand;
        if(event.rightHand != 0)
        {
            prevRightNote = event.rightHand;
            prevRightHit =0;
        }
        if(event.leftHand  != 0) 
        {
            prevLeftNote = event.leftHand;
            prevLeftHit =0;
        }
        
        events.push_back(event);
        num++;
    }

    for (const auto& e : events)
    {
        int rightFlag = (e.rightHand != 0) ? 1 : 0;
        int leftFlag  = (e.leftHand  != 0) ? 1 : 0;

        output << std::fixed << std::setprecision(3)
               << e.time
               << std::setw(6) << e.rightHand
               << std::setw(6) << e.leftHand
               << std::setw(6) << rightFlag
               << std::setw(6) << leftFlag
               << std::setw(6) << e.bassHit
               << std::setw(6) << e.hihat << "\n";
    }

    std::cout << "손 어사인 포함 변환 완료! 저장 위치 → " << outputFilename << "\n";
    std::cout << "전체 " << num << "줄 중 손 어사인 변경이 감지된 줄 수: " << changedCount << "줄\n";
    return 0;
}
