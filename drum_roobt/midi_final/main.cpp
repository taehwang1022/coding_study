#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>

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

void handleMetaEvent(const std::vector<unsigned char>& data, size_t& pos, int &initial_setting_flag) {
    unsigned char metaType = data[pos++];
    int length = static_cast<int>(data[pos++]);
    size_t startPos = pos;
    if (metaType == 0x21 && length == 1) {
        initial_setting_flag = 1;
    } else if (metaType == 0x58 && length == 4) {
        unsigned char numerator = data[pos];
        unsigned char denominator = 1 << data[pos + 1];
        std::cout << "  - Time Signature: " << (int)numerator << "/" << (int)denominator << "\n";
    } else if (metaType == 0x51 && length == 3) {
        int tempo = ((data[pos] & 0xFF) << 16) |
                    ((data[pos + 1] & 0xFF) << 8) |
                    (data[pos + 2] & 0xFF);
        int bpm = 60000000 / tempo;
        std::cout << "  - Tempo Change: " << bpm << " BPM\n";
    } else if (metaType == 0x2F) {
        std::cout << "  - End of Track reached\n";
    }
    pos = startPos + length;
}

void handleChannel10(const std::vector<unsigned char>& data, size_t& pos, unsigned char eventType) {
    unsigned char control = data[pos++];
    if (eventType == 0xB9) pos++;
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
        case 42: mappedDrumNote = 5; break;
        case 51: mappedDrumNote = 6; break;
        case 49: mappedDrumNote = 7; break;
        case 57: mappedDrumNote = 8; break;
        case 36: mappedDrumNote = 10; break;
        case 46: mappedDrumNote = 11; break;
        default: mappedDrumNote = 0; break;
    }
    file << note_on_time << "\t " << mappedDrumNote << "\n";
    file.close();
    note_on_time = 0;
}

void handleNoteOn(const std::vector<unsigned char>& data, size_t& pos, double &note_on_time, int tpqn, const std::string& midiFilePath) {
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
        note_on_time = ((note_on_time * 60000) / (100 * tpqn)) / 1000;
        std::cout << std::fixed << std::setprecision(1) << note_on_time << "s\t" << "Hit Drum: " << drumName << " -> " << (int)drumNote << "\n";
        save_to_csv(midiFilePath, note_on_time, (int)drumNote);
    }
}

void analyzeMidiEvent(const std::vector<unsigned char>& data, size_t& pos, unsigned char& runningStatus, int &initial_setting_flag, double &note_on_time, int &tpqn, const std::string& midiFilePath) {
    if (pos >= data.size()) return;
    unsigned char eventType = data[pos];
    if (eventType == 0xFF || eventType == 0xB9 || eventType == 0xC9 || eventType == 0x99) {
        runningStatus = eventType;
        pos++;
    } else {
        eventType = runningStatus;
    }
    if (eventType == 0xFF) {
        handleMetaEvent(data, pos, initial_setting_flag);
    } else if (eventType == 0xB9 || eventType == 0xC9) {
        handleChannel10(data, pos, eventType);
    } else if (eventType == 0x99) {
        handleNoteOn(data, pos, note_on_time, tpqn, midiFilePath);
    } else {
        pos++;
    }
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
                hihatState = 0;
            } else if (note == 5) {
                hihatState = 1;
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
}

static int zoneOf(int inst) {
    if (inst == 0) return 0;          // 비어있음
    if (inst == 5) return 1;          // 하이햇
    if (inst == 8 || inst == 4 || inst == 1) return 2; // 크래시(8), 하이탐(4), 스네어(1)
    if (inst == 2 || inst == 3 || inst == 6) return 3; // 플로어(2), 미드탐(3), 라이드벨(6)
    if (inst == 7) return 4;          // 라이드(7)
    return 3; // 정의 밖은 기본적으로 중앙-우측 계열로 가정
}

static bool isCrossed(int rightInst, int leftInst) {
    if (rightInst == 0 || leftInst == 0) return false;          // 한 손 비어있으면 꼬임 아님
    if (rightInst == 5 && leftInst == 1) return false;          // 예외 허용(오른손 하이햇, 왼손 스네어)
    int zr = zoneOf(rightInst);
    int zl = zoneOf(leftInst);
    return (zl > zr);
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
    int prevRight = 0, prevLeft = 0;
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

        if (inst1 == 7 || inst2 == 7) {
            e.rightHand = (inst1 == 7) ? 7 : inst2;
            e.leftHand = (inst1 == 7) ? inst2 : inst1;
        } else if (inst1 != 0 && inst2 != 0) {
            if (inst1 == 1 || inst2 == 1) {
                e.leftHand = (inst1 == 1) ? inst1 : inst2;
                e.rightHand = (inst1 == 1) ? inst2 : inst1;
            } else {
                e.rightHand = (inst1 == 2 || inst1 == 3 || inst1 == 6 || inst1 == 7) ? inst1 : inst2;
                e.leftHand = (e.rightHand == inst1) ? inst2 : inst1;
            }
        } else if (inst1 != 0) {
            if (inst1 == prevRight || inst1 == prevLeft) {
                if (e.time <= 0.1) {
                    e.rightHand = (inst1 == prevRight) ? inst1 : 0;
                    e.leftHand = (inst1 == prevLeft) ? inst1 : 0;
                } else {
                    double dMax = 0.754;
                    double dRight = 1.0, dLeft = 1.0; // simplified for this context
                    double tRight = prevRightHit * 1.38;
                    double tLeft = prevLeftHit * 1.38;
                    double rScore = (tRight/0.6) * (1 - std::min(dRight/dMax, 1.0));
                    double lScore = (tLeft/0.6) * (1 - std::min(dLeft/dMax, 1.0));
                    if (lScore <= rScore) e.rightHand = inst1;
                    else e.leftHand = inst1;
                }
            } else {
                if (inst1 == 2 || inst1 == 3 || inst1 == 6 || inst1 == 7) e.rightHand = inst1;
                else e.leftHand = inst1;
            }
        }


        // 손 배정이 끝난 직후 손 크로스 안되게 막는것
        if (e.rightHand != 0 && e.leftHand != 0) {
            int zr = zoneOf(e.rightHand);
            int zl = zoneOf(e.leftHand);

            // 예외: 오른손=하이햇(5), 왼손=스네어(1)
            bool exception = (e.rightHand == 5 && e.leftHand == 1);

            if (!exception && zl > zr) {
                std::swap(e.rightHand, e.leftHand);
            }
        }


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

    std::cout << "손 어사인 포함 변환 완료! 저장 위치 → " << outputFilename << "\n";
}

// 박자 단위 분할 및 마디 번호 부여 함수
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

        double remaining = ev.time;
        while (remaining > 0.6) {
            DrumEvent mid{0.6, 0, 0, 0, 0, 0, 0};
            result.push_back(mid);
            remaining -= 0.6;
        }
        if (remaining > 0.0) {
            ev.time = remaining;
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
    output << "-1" << "\t 0.600\t 1\t 1\t 1\t 1\t 1\t 1\n";

    std::cout << "코드 끗." << std::endl;
}


int main() {
    std::string midiNameOnly;
    size_t pos;
    unsigned char runningStatus;
    int initial_setting_flag = 0;
    double note_on_time = 0;

    std::cout << "Enter MIDI file name (without .mid): ";
    std::cin >> midiNameOnly;

    std::cout << "-------------------- midi to mc start --------------------" << std::endl;

    std::string inputPath  = "/home/taehwang/basic-algo-lecture-master/drum_roobt/midi_final/" + midiNameOnly + ".mid";
    std::string outputPath = "/home/taehwang/basic-algo-lecture-master/drum_roobt/midi_final/" + midiNameOnly + "_mc.csv";

    std::vector<unsigned char> midiData;
    if (!readMidiFile(inputPath, midiData)) return 1;

    pos = 14;
    int tpqn = (midiData[12] << 8) | midiData[13];
    std::cout << "Time Division (TPQN): " << tpqn << " ticks per quarter note\n";

    while (pos + 8 <= midiData.size()) {
        if (!(midiData[pos] == 'M' && midiData[pos+1] == 'T' && midiData[pos+2] == 'r' && midiData[pos+3] == 'k')) {
            std::cerr << "MTrk expected at pos " << pos << "\n";
            break;
        }
        size_t trackLength = (midiData[pos+4] << 24) |
                             (midiData[pos+5] << 16) |
                             (midiData[pos+6] << 8) |
                             midiData[pos+7];
        pos += 8;
        size_t trackEnd = pos + trackLength;

        std::cout << "🎵 Reading MTrk (length=" << trackLength << ") from pos " << pos << " to " << trackEnd << "\n";

        note_on_time = 0;
        while (pos < trackEnd) {
            size_t delta = readTime(midiData, pos);
            note_on_time += delta;
            analyzeMidiEvent(midiData, pos, runningStatus, initial_setting_flag, note_on_time, tpqn, outputPath);
        }
        pos = trackEnd;
    }

    std::cout << "-------------------- midi to mc done --------------------" << std::endl;
    std::cout << "--------------------- mc to c start ---------------------" << std::endl;

    // === mc to c ===
    convertMcToC(
        "/home/taehwang/basic-algo-lecture-master/drum_roobt/midi_final/" + midiNameOnly + "_mc.csv",
        "/home/taehwang/basic-algo-lecture-master/drum_roobt/midi_final/" + midiNameOnly + "_c.csv"
    );
    
    std::cout << "--------------------- mc to c done ---------------------" << std::endl;
    std::cout << "---------------- c to hand assign start ----------------" << std::endl;

    assignHandsToEvents(
        "/home/taehwang/basic-algo-lecture-master/drum_roobt/midi_final/" + midiNameOnly + "_c.csv",
        "/home/taehwang/basic-algo-lecture-master/drum_roobt/midi_final/" + midiNameOnly + "_a.csv"
    );

    std::cout << "---------------- hand assign to final done ----------------" << std::endl;

    convertToMeasureFile(
        "/home/taehwang/basic-algo-lecture-master/drum_roobt/midi_final/" + midiNameOnly + "_a.csv",
        "/home/taehwang/basic-algo-lecture-master/drum_roobt/midi_final/" + midiNameOnly + "_final.txt"
    );

    return 0;
}