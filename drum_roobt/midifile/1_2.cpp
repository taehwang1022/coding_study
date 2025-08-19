#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>

// ======================== midi2code_1.cpp 기능 ========================
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
        case 47: mappedDrumNote = 4; break;
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

    if (velocity > 0) {
        note_on_time = ((note_on_time * 60000) / (100 * tpqn)) / 1000;
        save_to_csv(midiFilePath, note_on_time, (int)drumNote);
    }
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
    pos++; // skip control
    if (eventType == 0xB9) pos++;
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

    if (eventType == 0xFF) handleMetaEvent(data, pos, initial_setting_flag);
    else if (eventType == 0xB9 || eventType == 0xC9) handleChannel10(data, pos, eventType);
    else if (eventType == 0x99) handleNoteOn(data, pos, note_on_time, tpqn, midiFilePath);
}

void convertMidiToCsv(const std::string& midiNameOnly) {
    std::string inputPath  = "/home/taehwang/basic-algo-lecture-master/drum_roobt/midifile/midbox/" + midiNameOnly + ".mid";
    std::string outputPath = "/home/taehwang/basic-algo-lecture-master/drum_roobt/midifile/mid2midcode/" + midiNameOnly + "_mc.csv";

    std::vector<unsigned char> midiData;
    if (!readMidiFile(inputPath, midiData)) return;

    size_t pos = 22;
    unsigned char runningStatus;
    int initial_setting_flag = 0;
    double note_on_time = 0;
    int tpqn = (midiData[12] << 8) | midiData[13];

    std::cout << "Time Division (TPQN): " << tpqn << "\n";
    while (pos < midiData.size()) {
        size_t time = readTime(midiData, pos);
        note_on_time += time;
        analyzeMidiEvent(midiData, pos, runningStatus, initial_setting_flag, note_on_time, tpqn, outputPath);
    }
    std::cout << "MIDI 변환 완료!\n";
}

// ======================== midicode2code_2.cpp 기능 ========================
struct Event {
    double time;
    std::vector<int> notes;
};

std::vector<std::string> splitByWhitespace(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string temp;
    while (iss >> temp) tokens.push_back(temp);
    return tokens;
}

void mergeCsvToFinal(const std::string& fileBaseName) {
    std::string inputFilename  = "/home/taehwang/basic-algo-lecture-master/drum_roobt/midifile/mid2midcode/" + fileBaseName + "_mc.csv";
    std::string outputFilename = "/home/taehwang/basic-algo-lecture-master/drum_roobt/midifile/RLassign/" + fileBaseName + "_c.csv";

    std::ifstream input(inputFilename);
    std::ofstream output(outputFilename);

    if (!input.is_open() || !output.is_open()) {
        std::cerr << "파일 입출력 실패\n";
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
        int inst1 = 0, inst2 = 0, bassHit = 0, hihat = hihatState;

        for (int note : e.notes) {
            if (note >= 1 && note <= 8) {
                if (inst1 == 0) inst1 = note;
                else if (inst2 == 0) inst2 = note;
            } else if (note == 10) bassHit = 1;
            else if (note == 11) hihatState = 0;
            else if (note == 5) hihatState = 1;
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

    std::cout << "병합 CSV 생성 완료!\n";
}

// ======================== main ========================
int main() {
    std::string fileBaseName;
    std::cout << "MIDI 파일 이름 입력 (.mid 제외): ";
    std::cin >> fileBaseName;

    convertMidiToCsv(fileBaseName);     // midi2code_1 기능 수행
    mergeCsvToFinal(fileBaseName);      // midicode2code_2 기능 수행

    std::cout << "✅ 변환 전체 완료!\n";
    return 0;
}
