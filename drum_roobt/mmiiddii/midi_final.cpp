#include <iostream>
#include <bits/stdc++.h>
#include <filesystem>

enum Hand { LEFT, RIGHT, SAME };

struct Coord {
    double x, y, z;
};

struct Event{
    double time;
    std::vector<int> notes;
};

struct VelocityEntry {
    double time;
    int instrument;
    int velocity;
};

Coord drumXYZ[9] = {
    {0.0, 0.0, 0.0},
    {-0.13, 0.52, 0.61}, {0.25, 0.50, 0.62}, {0.21, 0.67, 0.87},
    {-0.05, 0.69, 0.83}, {-0.28, 0.60, 0.88}, {0.32, 0.71, 1.06},
    {0.47, 0.52, 0.88}, {-0.06, 0.73, 1.06}
};

std::vector<std::string> splitByWhitespace(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) tokens.push_back(token);
    return tokens;
}

void handleMetaEvent(const std::vector<unsigned char>& data, size_t& pos, int &bpm) {
    unsigned char metaType = data[pos++];
    int length = static_cast<int>(data[pos++]);
    size_t startPos = pos;
    if (metaType == 0x21 && length == 1) {
    } else if (metaType == 0x58 && length == 4) {
        unsigned char numerator = data[pos];
        unsigned char denominator = 1 << data[pos + 1];
        // std::cout << "  - Time Signature: " << (int)numerator << "/" << (int)denominator << "\n";
    } else if (metaType == 0x51 && length == 3) {
        int tempo = ((data[pos] & 0xFF) << 16) |
                    ((data[pos + 1] & 0xFF) << 8) |
                    (data[pos + 2] & 0xFF);
        bpm = 60000000 / tempo;
        bpm =100;
        std::cout << "  - Tempo Change: " << bpm << " BPM\n";
        usleep(5000000);
    } else if (metaType == 0x2F) {
        // std::cout << "  - End of Track reached\n";
    }
    pos = startPos + length;
}

void handleChannel10(const std::vector<unsigned char>& data, size_t& pos, unsigned char eventType) {
    unsigned char control = data[pos++];
    if (eventType == 0xB9) pos++;
}

bool readMidiFile(const std::string& filename, std::vector<unsigned char>& buffer) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return false;
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    buffer.resize(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cerr << "Failed to read file: " << filename << std::endl;
        return false;
    }
    return true;
}

size_t readTime(const std::vector<unsigned char>& data, size_t& pos) {
    size_t value = 0;
    while (pos < data.size()) {
        unsigned char byte = data[pos];
        value = (value << 7) | (byte & 0x7F);
        pos++;
        if ((byte & 0x80) == 0) break;
    }
    return value;
}

void save_to_csv(const std::string& outputCsvPath, double &note_on_time, int drumNote) {
    std::ofstream file(outputCsvPath, std::ios::app);
    if (!file) {
        std::cerr << "Failed to open CSV file: " << outputCsvPath << std::endl;
        return;
    }
    int mappedDrumNote;
    switch (drumNote) {
        case 38: mappedDrumNote = 1; break;
        case 41: mappedDrumNote = 2; break;
        case 45: mappedDrumNote = 3; break;
        case 47: case 48: case 50: mappedDrumNote = 4; break;
        case 46: mappedDrumNote = 5; break;
        case 51: mappedDrumNote = 6; break;
        case 49: mappedDrumNote = 8; break;
        case 57: mappedDrumNote = 7; break;
        case 36: mappedDrumNote = 10; break;
        case 42: mappedDrumNote = 11; break;
        default: mappedDrumNote = 0; break;
    }
    file << note_on_time << "\t " << mappedDrumNote << "\n";
    file.close();
    note_on_time = 0;
}

void handleNoteOn(const std::vector<unsigned char>& data, size_t& pos, double &note_on_time, int tpqn, int bpm, const std::string& midiFilePath) {
    if (pos + 2 > data.size()) return;
    unsigned char drumNote = data[pos++];
    unsigned char velocity = data[pos++];
    std::string drumName;
    switch ((int)drumNote) {
        case 36: drumName = "Bass Drum 1"; break;
        case 41: drumName = "Low Floor Tom"; break;
        case 38: drumName = "Acoustic Snare"; break;
        case 45: drumName = "Low Tom"; break;
        case 47: case 48: case 50: drumName = "Low Mid Tom"; break;
        case 42: drumName = "Closed Hi-Hat"; break;
        case 46: drumName = "Open Hi-Hat"; break;
        case 49: drumName = "Crash Cymbal 1"; break;
        case 51: drumName = "Ride Cymbal 1"; break;
        case 57: drumName = "Crash Cymbal 2"; break;
        default: drumName = "Unknown Drum"; break;
    }
    if (velocity > 0) {
        note_on_time = ((note_on_time * 60000) / (bpm * tpqn)) / 1000;
        // std::cout << std::fixed << std::setprecision(1) << note_on_time << "s\t" << "Hit Drum: " << drumName << " -> " << (int)drumNote << "\n";
        save_to_csv(midiFilePath, note_on_time, drumNote);
    }
}

void analyzeMidiEvent(const std::vector<unsigned char>& data, size_t& pos, unsigned char& runningStatus, double &note_on_time, int &tpqn, int &bpm, const std::string& midiFilePath) {
    if (pos >= data.size()) return;
    unsigned char eventType = data[pos];
    if (eventType == 0xFF || eventType == 0xB9 || eventType == 0xC9 || eventType == 0x99 || eventType == 0x89|| eventType == 0xA9) {
        runningStatus = eventType;
        pos++;
    } else {
        eventType = runningStatus;
    }
    if (eventType == 0xFF) {
        handleMetaEvent(data, pos, bpm);
    } else if (eventType == 0xB9 || eventType == 0xC9) {
        handleChannel10(data, pos, eventType);
    } else if (eventType == 0x99) {
        handleNoteOn(data, pos, note_on_time, tpqn, bpm, midiFilePath);
    } else if (eventType == 0x89 || eventType == 0xA9) {
        pos += 2;
    }else {
        std::cout << "unknown midiEvent - analyzeMidiEvent\n";
        pos++;
    }
}

void roundDurationsToStep(const std::string& inputFilename, const std::string& outputFilename)
{
    std::ifstream inputFile(inputFilename);
    std::ofstream outputFile(outputFilename);

    if (!inputFile.is_open()) {
        std::cerr << "roundDurationsToStep 입력 파일 열기 실패: " << inputFilename << std::endl;
        return;
    }

    if (!outputFile.is_open()) {
        std::cerr << "roundDurationsToStep 출력 파일 열기 실패: " << outputFilename << std::endl;
        return;
    }

    std::string line;
    const double step = 0.05;

    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        double duration;
        int note;

        if (!(iss >> duration >> note)) {
            std::cerr << "roundDurationsToStep 잘못된 형식: " << line << std::endl;
            continue;
        }

        // 0.05 단위로 반올림
        double roundedDuration = std::round(duration / step) * step;

        outputFile << std::fixed << std::setprecision(3)
                   << roundedDuration << "\t" << note << std::endl;
    }

    inputFile.close();
    outputFile.close();

}

double dist(const Coord& a, const Coord& b) {
    return std::sqrt(
        (a.x - b.x)*(a.x - b.x) +
        (a.y - b.y)*(a.y - b.y) +
        (a.z - b.z)*(a.z - b.z)
    );
}

Hand getPreferredHandByDistance(int instCurrent, int prevRightNote, int prevLeftNote, double prevRightHit, double prevLeftHit) {

    Coord curr = drumXYZ[instCurrent];
    Coord right = drumXYZ[prevRightNote];
    Coord left = drumXYZ[prevLeftNote];

    double dMax = 0.754;
    double dRight = dist(curr, right);
    double dLeft = dist(curr, left);
    // 시간의 max값 0.6으로 고정
    double real_tRight = std::min(prevRightHit, 0.6) * 1.38;
    double real_tLeft  = std::min(prevLeftHit, 0.6) * 1.38;
    double normRight = std::min(dRight / dMax, 1.0);
    double normLeft = std::min(dLeft / dMax, 1.0);

    double rScore = (real_tRight / 0.6) * (1 - normRight);
    double lScore = (real_tLeft  / 0.6) * (1 - normLeft);

    // 디버깅용 프린트문
    std::cout << "\n[Hand 선택 판단]\n";
    std::cout << " - instCurrent: " << instCurrent << "\n";
    std::cout << " - prevRightNote: " << prevRightNote << ", prevLeftNote: " << prevLeftNote << "\n";
    std::cout << " - 거리: right = " << dRight << ", left = " << dLeft << "\n";
    std::cout << " - 시간누적: right = " << prevRightHit << ", left = " << prevLeftHit << "\n";
    std::cout << " - 정규화 거리: right = " << normRight << ", left = " << normLeft << "\n";
    std::cout << " - 점수: rScore = " << rScore << ", lScore = " << lScore << "\n";

    if (std::abs(rScore - lScore) < 1e-6) {
        std::cout << " → 결과: SAME (유사한 점수)\n";
        return SAME;
    }

    Hand chosen = (lScore <= rScore) ? RIGHT : LEFT;
    std::cout << " → 선택된 손: " << (chosen == RIGHT ? "RIGHT" : "LEFT") << "\n";
    return chosen;
}

void convertMcToC(const std::string& inputFilename, const std::string& outputFilename) {
    std::ifstream input(inputFilename);
    if (!input.is_open()) {
        std::cerr << " 입력 파일 열기 실패: " << inputFilename << "\n";
        return;
    }
    std::ofstream output(outputFilename);
    if (!output.is_open()) {
        std::cerr << " 출력 파일 생성 실패: " << outputFilename << "\n";
        return;
    }

    std::vector<Event> mergedEvents;  
    std::string line;
    double currentTime = 0.0;
    int hihatState = 1;

    while (std::getline(input, line)) {
        auto tokens = splitByWhitespace(line);
        if (tokens.size() != 2) continue;
        try {
            double delta = std::stod(tokens[0]);
            int rawNote = std::stoi(tokens[1]);
            int mapped = rawNote;
            if (mapped < 1 || mapped > 11) continue;
            if (mergedEvents.empty() || delta > 0) {
                currentTime += delta;
                mergedEvents.push_back({currentTime, {mapped}});
            } else {
                mergedEvents.back().notes.push_back(mapped);
            }
        } catch (...) { continue; }
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
                if (inst1 == 0) inst1 = 5;
                else if (inst2 == 0) inst2 = 5;
                hihatState = 1;
            } else if (note == 5) {
                hihatState = 0;
                if (inst1 == 0) inst1 = 5;
                else if (inst2 == 0) inst2 = 5;
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

    // std::cout << "변환 완료! 저장 위치 → " << outputFilename << "\n";
}

std::pair<int, int> assignHandsByPosition(int inst1, int inst2) {
    auto getSection = [](int inst) {
        if (inst == 5) return 1;
        if (inst == 1 || inst == 4 || inst == 8) return 2;
        if (inst == 2 || inst == 3 || inst == 6) return 3;
        if (inst == 7) return 4;
        return 0; // unknown
    };

    auto getSectionOrder = [](int inst) {
        // 낮을수록 왼쪽
        if (inst == 8) return 1;
        if (inst == 1) return 2;
        if (inst == 4) return 3;
        if (inst == 3) return 1;
        if (inst == 2) return 2;
        if (inst == 6) return 3;
        return 0;
    };

    int sec1 = getSection(inst1);
    int sec2 = getSection(inst2);

    std::cout << "[섹션 손 분배 판단]\n";
    std::cout << " - inst1: " << inst1 << " (섹션 " << sec1 << "), inst2: " << inst2 << " (섹션 " << sec2 << ")\n";

    if (sec1 < sec2) {
        std::cout << " → 서로 다른 섹션: inst1이 더 왼쪽 → 왼손 = " << inst1 << ", 오른손 = " << inst2 << "\n";
        return {inst1, inst2};
    } else if (sec2 < sec1) {
        std::cout << " → 서로 다른 섹션: inst2가 더 왼쪽 → 왼손 = " << inst2 << ", 오른손 = " << inst1 << "\n";
        return {inst2, inst1};
    } else {
        int order1 = getSectionOrder(inst1);
        int order2 = getSectionOrder(inst2);
        std::cout << " → 같은 섹션 내 비교: order1 = " << order1 << ", order2 = " << order2 << "\n";
        if (order1 < order2) {
            std::cout << "   → inst1이 더 왼쪽 → 왼손 = " << inst1 << ", 오른손 = " << inst2 << "\n";
            return {inst1, inst2};
        } else {
            std::cout << "   → inst2가 더 왼쪽 → 왼손 = " << inst2 << ", 오른손 = " << inst1 << "\n";
            return {inst2, inst1};
        }
    }
}

void assignHandsToEvents(const std::string& inputFilename, const std::string& outputFilename) {
    std::ifstream input(inputFilename);
    if (!input.is_open()) {
        std::cerr << "입력 파일 열기 실패: " << inputFilename << "\n";
        return;
    }
    std::ofstream output(outputFilename);
    if (!output.is_open()) {
        std::cerr << "출력 파일 생성 실패: " << outputFilename << "\n";
        return;
    }

    struct FullEvent {
        double time;
        int inst1 = 0, inst2 = 0, bassHit = 0, hihat = 1;
        int rightHand = 0, leftHand = 0;
    };

    std::string line;
    std::vector<FullEvent> events;
    int prevRight = 1, prevLeft = 1;
    int prevRightNote = 1, prevLeftNote = 1;
    double prevRightHit = 0, prevLeftHit = 0;

    while (std::getline(input, line)) {
        auto tokens = splitByWhitespace(line);
        if (tokens.size() != 7) continue;

        FullEvent e;
        e.time = std::stod(tokens[0]);
        e.inst1 = std::stoi(tokens[1]);
        e.inst2 = std::stoi(tokens[2]);
        e.bassHit = std::stoi(tokens[5]);
        e.hihat = std::stoi(tokens[6]);

        int inst1 = e.inst1, inst2 = e.inst2;
        prevRightHit += e.time;
        prevLeftHit += e.time;

        std::cout << "[Time: " << e.time << "] inst1: " << inst1 << ", inst2: " << inst2
                  << " | PrevR: " << prevRight << ", PrevL: " << prevLeft
                  << " | RHit: " << prevRightHit << ", LHit: " << prevLeftHit << "\n";
        
        //step 1 크러시가 있는지 확인 크러쉬가 있다면 
        if (inst1 == 8 || inst2 == 8) {
            std::cout << "→ 크러시 처리 진입\n";
            if(prevLeft == 2 || prevLeft == 3 || prevLeft == 6) {
                e.rightHand = 7;
                e.leftHand = (inst1 == 8) ? inst2 : inst1;
            } else {
                if (inst1 == 2 || inst1 == 3 || inst1 == 6 || inst2 == 2 || inst2 == 3 || inst2 == 6) {
                    e.rightHand = 7;
                    e.leftHand = (inst1 == 8) ? inst2 : inst1;
                } else {
                    e.rightHand = 8;
                    e.leftHand = (inst1 == 8) ? inst2 : inst1;
                }
            }
        }
        // step 2 양손 연주인지 한손인지 구분 
        else if (inst1 != 0 && inst2 != 0) {
            std::cout << "→ 양손 처리 진입\n";
            // 1번 S와 5번 H-H 을 같이 치는 경우 오른손으로 H-H 왼손으로 S 치도록 설정
            if ((inst1 == 5 && inst2 == 1) || (inst1 == 1 && inst2 == 5)) 
            {
                e.leftHand = (inst1 == 5) ? inst2 : inst1;
                e.rightHand = (inst1 == 5) ? inst1 : inst2;
            }
            // 위의 경우를 제외한 모든 경우 악기 위치 기반으로 손 분배 
            else 
            {
                auto [left, right] = assignHandsByPosition(inst1,inst2);
                e.leftHand = left;
                e.rightHand = right;
            }
        }
        // step 3 한손 연주시 처리 
        else if (inst1 != 0) {
            std::cout << "→ 한손 처리 진입\n";
            // 이전에 쳤던 악기와 같은 악기가 감지된다면 짧은 시간에 타격해야 할 시 같은 손 유지 시간차이가 크다면 거리 기반 판단
            if (inst1 == prevRight || inst1 == prevLeft) {
                std::cout << "    - 이전 손과 같은 악기 감지\n";
                if (e.time <= 0.1) {
                    std::cout << "    - 시간차 0.1 이하 → 같은 손 유지\n";
                    if(inst1 == prevRight)
                    {
                        e.rightHand = inst1;
                        e.leftHand = 0;
                    }
                    else
                    {
                        e.rightHand = 0;
                        e.leftHand = inst1;
                    }
                } else {
                    std::cout << "    - 시간차 큼 → 거리 기반 판단\n";
                    Hand preferred = getPreferredHandByDistance(inst1, prevRightNote, prevLeftNote, prevRightHit, prevLeftHit);
                    if (preferred == RIGHT) {
                        std::cout << "    → RIGHT 선택\n";
                        e.rightHand = inst1;
                        e.leftHand = 0;
                    } else if (preferred == LEFT) {
                        std::cout << "    → LEFT 선택\n";
                        e.leftHand = inst1;
                        e.rightHand = 0;
                    }    
                    else {
                        std::cout << "    → 점수 같고 시간 널널 오른손 우선권 선택\n";
                        // 여기는 한손 연주이면서 전에 쳤던 악기를 치는 것이지만 시간과 거리에 대한 점수도 모두 동일함 일단 오른손에 우선권을 주겠다.
                        e.rightHand = inst1;
                        e.leftHand = 0;
                    }
                }
            } else {
                std::cout << "    - 이전 손과 다른 악기 → 거리 기반 판단\n";
                Hand preferred = getPreferredHandByDistance(inst1, prevRightNote, prevLeftNote, prevRightHit, prevLeftHit);
                    if (preferred == RIGHT) {
                        std::cout << "    → RIGHT 선택\n";
                        e.rightHand = inst1;
                    } else if (preferred == LEFT) {
                        std::cout << "    → LEFT 선택\n";
                        e.leftHand = inst1;
                    } 
                    //악기 위치 거리 기반으로 손 분배 이때 전에 친악기를 inst2로 사용해서 구함  
                    else {
                        std::cout << "    → SAME 판단 → 섹션 기반 분배\n";
                        
                        int inst2 = (prevRightNote != 0) ? prevRightNote : prevLeftNote;
                        auto [left, right] = assignHandsByPosition(inst1, inst2);
                        if(left  == inst1)
                            e.leftHand  = (left  == inst1) ? inst1 : 0;
                        else
                            e.rightHand = (right == inst1) ? inst1 : 0;
                        
                        
                        std::cout << "      - inst1: " << inst1 << ", inst2: " << inst2
                                    << " → 왼손 = " << e.leftHand << ", 오른손 = " << e.rightHand << "\n";
                    }
            }
        }

        std::cout << "→ 결과: RH = " << e.rightHand << ", LH = " << e.leftHand << "\n\n";

        prevRight = e.rightHand;
        prevLeft = e.leftHand;
        if (e.rightHand != 0) { prevRightNote = e.rightHand; prevRightHit = 0; }
        if (e.leftHand != 0) { prevLeftNote = e.leftHand; prevLeftHit = 0; }

        events.push_back(e);
    }

    for (const auto& e : events) {
        int rightFlag = 0;
        int leftFlag = 0;
        if(e.rightHand != 0)    rightFlag = 5;
        if(e.leftHand != 0)     leftFlag = 5;
        output << std::fixed << std::setprecision(3)
               << e.time
               << std::setw(6) << e.rightHand
               << std::setw(6) << e.leftHand
               << std::setw(6) << rightFlag
               << std::setw(6) << leftFlag
               << std::setw(6) << e.bassHit
               << std::setw(6) << e.hihat << "\n";
    }
}

void convertToMeasureFile(const std::string& inputFilename, const std::string& outputFilename) {
    struct DrumEvent {
        double time;
        int rightInstrument;
        int leftInstrument;
        int rightPower;
        int leftPower;
        int isBass;
        int hihatOpen;
    };

    std::ifstream input(inputFilename);
    if (!input.is_open()) {
        std::cerr << "입력 파일 열기 실패: " << inputFilename << "\n";
        return;
    }
    std::ofstream output(outputFilename);
    if (!output.is_open()) {
        std::cerr << "출력 파일 생성 실패: " << outputFilename << "\n";
        return;
    }

    std::string line;
    std::vector<DrumEvent> result;

    while (std::getline(input, line)) {
        std::stringstream ss(line);
        DrumEvent ev;
        ss >> ev.time >> ev.rightInstrument >> ev.leftInstrument
           >> ev.rightPower >> ev.leftPower >> ev.isBass >> ev.hihatOpen;

        int count = static_cast<int>(ev.time / 0.6);
        double leftover = ev.time - count * 0.6;

        for (int i = 0; i < count; ++i) {
            DrumEvent mid{0.6, 0, 0, 0, 0, 0, 0};
            result.push_back(mid);
        }
        if (leftover > 1e-6) {
            ev.time = leftover;
            result.push_back(ev);
        }
    }

    output << "1\t 0.600\t 0\t 0\t 0\t 0\t 0\t 0\n";

    double measureTime = 0.0;
    int measureNum = 1;
    const double EPS = 1e-6;
    const double MEASURE_LIMIT = 2.4;

    for (const auto& ev : result) {
        measureTime += ev.time;
        // 마디 시간이 2.4초를 넘으면 새로운 마디로 시작
        if (measureTime >= MEASURE_LIMIT) {
            measureNum++;
            measureTime = ev.time;  // 새로운 마디 시간은 현재 이벤트의 시간으로 시작
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
    output << "-1" << "\t 0.600\t 1\t 1\t 1\t 1\t 1\t 1\n";
}

void addGroove(int bpm, const std::string& inputFile, const std::string& outputFile) {
    std::ifstream inFile(inputFile);
    std::ofstream outFile(outputFile);

    std::vector<std::vector<double>> values;
    std::vector<double> originalTimes;

        std::string line;
    // 1. 파일 읽기 및 파싱
    while (std::getline(inFile, line)) {
        std::istringstream ss(line);
        std::vector<double> row;
        double num;
        while (ss >> num) {
            row.push_back(num);
        }
        values.push_back(row);
        originalTimes.push_back(row[0]);  // 시간만 따로 저장
    }

    // 2. 누적합 기준 계산
    double threshold = (60.0 / bpm) * 2;
    double accTime = 0.0;

    // 3. 시간 조정 플래그 처리
    for (size_t i = 0; i < values.size(); ++i) {
        accTime += originalTimes[i];  // 항상 원본 기준 누적합 계산

        if (accTime >= threshold) {
            // 1. 현재 줄 시간값 -0.05
            values[i][0] -= 0.05;

            // 2. 다음 줄 존재하면 +0.05
            if (i + 1 < values.size()) {
                values[i + 1][0] += 0.05;
            }

            // 누적합 초기화
            accTime = 0.0;
        }
    }

    // 4. 결과 출력
    for (const auto& row : values) {

        outFile << std::fixed << std::setprecision(3) << row[0];
    

        outFile.unsetf(std::ios::fixed);
        outFile << std::setprecision(6);
    
        for (size_t i = 1; i < row.size(); ++i) {
            outFile << "\t" << row[i];
        }
        outFile << "\n";
    }

    
    inFile.close();
    outFile.close();
}

void addDynamics(int bpm, const std::string& inputFile, const std::string& outputFile,const std::string& velocityFile)
{
    std::ifstream inFile(inputFile);
    std::ifstream velocityData(velocityFile);
    std::ofstream outFile(outputFile);

    // velocityFile 의 형식은 한줄에 시간 악기 세기 순서로 
}

void MakeVelocitySummary(int bpm, const std::string& velocityFile,const std::string& outputFile)
{
    //windowSize는 한 마디의 시간
    double windowSize = (60/(double)bpm)*4;

    std::cout << windowSize << std::endl;

    std::ifstream in(velocityFile);

    std::vector<VelocityEntry> rawData;
    double t; int inst, vel;
    double maxTime = 0.0;

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;

        if (!std::getline(ss, token, ',')) continue;
        t = std::stod(token);
        if (!std::getline(ss, token, ',')) continue;
        inst = std::stoi(token);
        if (!std::getline(ss, token, ',')) continue;
        vel = std::stoi(token);

        rawData.push_back({t, inst, vel});
        if (t > maxTime) maxTime = t;
    }
    in.close();

    const int numWindows = static_cast<int>(std::ceil((maxTime + 0.001) / windowSize));

    std::vector<double> sumDrum(numWindows, 0.0);  // inst 1~4
    std::vector<int>    cntDrum(numWindows, 0);

    std::vector<double> sumCym(numWindows, 0.0);   // inst 5~8
    std::vector<int>    cntCym(numWindows, 0);

    // 시간 구간별 누적
    for (const auto& e : rawData) {
        int bin = static_cast<int>(e.time / windowSize);
        if (bin < 0 || bin >= numWindows) continue;

        if (e.instrument >= 1 && e.instrument <= 4) {
            sumDrum[bin] += e.velocity;
            cntDrum[bin]++;
        } else if (e.instrument >= 5 && e.instrument <= 8) {
            sumCym[bin] += e.velocity;
            cntCym[bin]++;
        }
    }

    // 평균 계산 → 40으로 나누고 반올림해 정수화 -> 127이 Max 이기 때문에 이런 방식을 채택
    std::vector<int> avgDrum(numWindows, 0);
    std::vector<int> avgCym(numWindows, 0);
    for (int i = 0; i < numWindows; ++i) {
        if (cntDrum[i] > 0) {
            double mean = sumDrum[i] / cntDrum[i];
            avgDrum[i] = static_cast<int>(std::round(mean / 40.0));
        }
        if (cntCym[i] > 0) {
            double mean = sumCym[i] / cntCym[i];
            avgCym[i] = static_cast<int>(std::round(mean / 40.0));
        }
    }

    // 결과 저장 (TSV)
    std::ofstream out(outputFile);
    out << "start_time\tend_time\tdrum_avg\tcymbal_avg\n";
    out << std::fixed << std::setprecision(3);
    for (int i = 0; i < numWindows; ++i) {
        double startT = i * windowSize;
        double endT   = (i + 1) * windowSize;
        out << startT << '\t' << endT << '\t'
            << avgDrum[i] << '\t' << avgCym[i] << '\n';
    }
    out.close();

    std::cout << "[완료] 드럼/심벌 평균 벨로시티 저장: " << outputFile << "\n";
}




int main() {

    std::string filename;
    std::cout << "읽을 MIDI 파일 이름을 입력하세요 (예: input0.mid): ";
    std::cin >> filename;

    size_t pos;
    unsigned char runningStatus;
    int initial_setting_flag = 0;
    double note_on_time = 0;
    std::vector<unsigned char> midiData;

    std::filesystem::path basePath = "/home/taehwang/basic-algo-lecture-master/drum_roobt/mmiiddii/";

    std::filesystem::path magentaPath = basePath / filename;
    
    std::string fileStem = filename.substr(0, filename.find_last_of('.'));  // ex: "input0"
    std::filesystem::path outputDir = basePath / "output" / fileStem;
    std::filesystem::create_directories(outputDir);
    
    std::filesystem::path outputPath1 = outputDir / "output1_drum_hits_time.csv"; 
    std::filesystem::path outputPath2 = outputDir / "output2_mc.csv";   
    std::filesystem::path outputPath3 = outputDir / "output3_mc2c.csv";    
    std::filesystem::path outputPath4 = outputDir / "output4_hand_assign.csv";
    std::filesystem::path outputPath5 = outputDir / "output5_add_groove.csv";
    std::filesystem::path outputPath6 = outputDir / ("output6_final_" + fileStem + ".txt");
    
    std::filesystem::path VelfileOrigin = basePath / "VelfileOrigin.csv";
    std::filesystem::path Velfile       = basePath / "Velfile.txt";
    

    if (!readMidiFile(magentaPath, midiData)) {
        std::cout << "mid file error\n";
        return 1;
    }

    pos = 14;
    int tpqn = (midiData[12] << 8) | midiData[13];
    int bpm;
    while (pos + 8 <= midiData.size()) {
        if (!(midiData[pos] == 'M' && midiData[pos+1] == 'T' && midiData[pos+2] == 'r' && midiData[pos+3] == 'k')) break;
        size_t trackLength = (midiData[pos+4] << 24) |
                             (midiData[pos+5] << 16) |
                             (midiData[pos+6] << 8) |
                             midiData[pos+7];
        pos += 8;
        size_t trackEnd = pos + trackLength;

        note_on_time = 0;
        while (pos < trackEnd) {
            size_t delta = readTime(midiData, pos);
            note_on_time += delta;
            analyzeMidiEvent(midiData, pos, runningStatus, note_on_time, tpqn, bpm, outputPath1);
        }
        pos = trackEnd;
    }



    MakeVelocitySummary(bpm,VelfileOrigin,Velfile);


    roundDurationsToStep(outputPath1, outputPath2); 
    convertMcToC(outputPath2, outputPath3);
    assignHandsToEvents(outputPath3, outputPath4);
    addGroove(bpm, outputPath4, outputPath5);
    convertToMeasureFile(outputPath5, outputPath6);
    
    return 0;
}
