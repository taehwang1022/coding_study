#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>

// ========================== 1단계: MIDI to midcode CSV ==========================
bool readMidiFile(const std::string& filename, std::vector<unsigned char>& buffer) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) return false;
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    buffer.resize(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return file.good();
}

size_t readTime(const std::vector<unsigned char>& data, size_t& pos) {
    size_t value = 0;
    while (pos < data.size()) {
        unsigned char byte = data[pos++];
        value = (value << 7) | (byte & 0x7F);
        if ((byte & 0x80) == 0) break;
    }
    return value;
}

void save_to_csv(const std::string& outputCsvPath, double time_in_sec, int drumNote) {
    std::ofstream file(outputCsvPath, std::ios::app);
    if (!file) {
        std::cerr << "CSV 파일 열기 실패\n";
        return;
    }

    int mappedDrumNote;
    switch (drumNote) {
        case 38: mappedDrumNote = 1; break;
        case 41: mappedDrumNote = 2; break;
        case 45: mappedDrumNote = 3; break;
        case 47: mappedDrumNote = 4; break;
        case 42: mappedDrumNote = 5; break;
        case 51: mappedDrumNote = 6; break;
        case 49: mappedDrumNote = 7; break;
        case 57: mappedDrumNote = 8; break;
        case 36: mappedDrumNote = 10; break;
        case 46: mappedDrumNote = 11; break;
        default: return;
    }

    file << std::fixed << std::setprecision(3)
         << time_in_sec << "\t" << mappedDrumNote << "\n";
}

void extract_midi_events(const std::string& midiFilePath, const std::string& outputCsvPath) {
    size_t pos = 22;
    unsigned char runningStatus;
    double tick_accum = 0;

    std::vector<unsigned char> midiData;
    if (!readMidiFile(midiFilePath, midiData)) {
        std::cerr << "파일 열기 실패: " << midiFilePath << std::endl;
        return;
    }

    int tpqn = (midiData[12] << 8) | midiData[13];
    std::cout << "Time Division (TPQN): " << tpqn << " ticks per quarter note\n";

    while (pos < midiData.size()) {
        size_t delta = readTime(midiData, pos);
        tick_accum += delta;

        if (pos >= midiData.size()) break;

        unsigned char eventType = midiData[pos];
        if (eventType < 0x80) {
            eventType = runningStatus;
        } else {
            runningStatus = eventType;
            pos++;
        }

        if (eventType == 0x99 && pos + 2 <= midiData.size()) {
            unsigned char drumNote = midiData[pos++];
            unsigned char velocity = midiData[pos++];
            if (velocity > 0) {
                double time_in_sec = ((delta * 60000.0) / (100.0 * tpqn)) / 1000.0;
                save_to_csv(outputCsvPath, time_in_sec, drumNote);
            }
        }
    }

    std::cout << "MIDI → CSV 변환 완료: " << outputCsvPath << "\n";
}

// ========================== 2단계: midcode CSV → 병합 CSV ==========================
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

void merge_events_and_save(const std::string& inputCsvPath, const std::string& outputCsvPath) {
    int hihatState = 1;
    std::ifstream input(inputCsvPath);
    std::ofstream output(outputCsvPath);

    if (!input.is_open()) {
        std::cerr << " 입력 파일 열기 실패: " << inputCsvPath << "\n";
        return;
    }

    if (!output.is_open()) {
        std::cerr << " 출력 파일 생성 실패: " << outputCsvPath << "\n";
        return;
    }

    std::vector<Event> mergedEvents;
    std::string line;
    double currentTime = 0.0;

    while (std::getline(input, line)) {
        auto tokens = splitByWhitespace(line);
        if (tokens.size() != 2) continue;

        try {
            double delta = std::stod(tokens[0]);
            int note = std::stoi(tokens[1]);

            if (note < 1 || note > 11) continue;

            if (mergedEvents.empty() || delta > 0) {
                currentTime += delta;
                mergedEvents.push_back({currentTime, {note}});
            } else {
                mergedEvents.back().notes.push_back(note);
            }
        } catch (...) {
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
               << deltaTime << "\t" << inst1 << "\t" << inst2
               << "\t0\t0\t" << bassHit << "\t" << hihat << "\n";
    }

    std::cout << "병합 CSV 생성 완료: " << outputCsvPath << "\n";
}

// ========================== main ==========================
int main() {
    std::string baseName;
    std::cout << "MIDI 파일 이름 입력 (.mid 제외): ";
    std::cin >> baseName;

    std::string inputMidiPath  = "./midbox/" + baseName + ".mid";
    std::string outputCsv1     = "./" + baseName + "_mc.csv";
    std::string outputCsv2     = "./" + baseName + "_c.csv";

    extract_midi_events(inputMidiPath, outputCsv1);     // 1단계
    merge_events_and_save(outputCsv1, outputCsv2);      // 2단계

    std::cout << "✅ 1번 + 2번 기능 통합 완료\n";
    return 0;
}
