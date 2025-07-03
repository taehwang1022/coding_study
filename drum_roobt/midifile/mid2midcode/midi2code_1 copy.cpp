#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>


// Function Declarations
bool readMidiHeader(const std::vector<unsigned char>& data);
size_t readTime(const std::vector<unsigned char>& data, size_t& pos);
void analyzeMidiEvent(const std::vector<unsigned char>& data, size_t& pos, unsigned char& runningStatus, int &initial_setting_flag, double &note_on_time, int &tpqn);
void handleMetaEvent(const std::vector<unsigned char>& data, size_t& pos,int &initial_setting_flag);
void handleChannel10(const std::vector<unsigned char>& data, size_t& pos, unsigned char eventType);
void handleNoteOn(const std::vector<unsigned char>& data, size_t& pos, double &note_on_time,int tpqn, const std::string& midiFilePath);
bool readMidiFile(const std::string& filename, std::vector<unsigned char>& buffer);
void save_to_csv(const std::string& midiFilePath, double &note_on_time, int drumNote);

// Read Variable Length Quantity (VLQ) for delta time
size_t readTime(const std::vector<unsigned char>& data, size_t& pos) {
    size_t value = 0;

    while (pos < data.size()) {
        unsigned char byte = data[pos];
        value = (value << 7) | (byte & 0x7F); // Use only the lower 7 bits

        pos++; // Move to the next byte

        if ((byte & 0x80) == 0) { // Stop if MSB is 0
            break;
        }
    }

    return value;
}

// Process MIDI events with Running Status support
void analyzeMidiEvent(const std::vector<unsigned char>& data, size_t& pos, unsigned char& runningStatus, int &initial_setting_flag, double &note_on_time, int &tpqn, const std::string& midiFilePath) {
    if (pos >= data.size()) return;

    unsigned char eventType = data[pos];

    // Update Running Status only for valid event types
    if (eventType == 0xFF || eventType == 0xB9 || eventType == 0xC9 || eventType == 0x99) {  
        runningStatus = eventType;
        pos++; // Consume event byte
    } else {
        // Retain Running Status if event type is missing
        eventType = runningStatus;
    }

    // Handle Meta Event
    if (eventType == 0xFF) { 
        handleMetaEvent(data, pos,initial_setting_flag);
    }
    // Handle Channel 10 events (0xB9: Control Change, 0xC9: Program Change)
    else if (eventType == 0xB9 || eventType == 0xC9) { 
        handleChannel10(data, pos, eventType);
    }
    // Handle Note On events (0x99)
    else if (eventType  == 0x99) { 
        handleNoteOn(data, pos, note_on_time,tpqn,midiFilePath);
    }
}
// Process Meta Events (Essential Only)
void handleMetaEvent(const std::vector<unsigned char>& data, size_t& pos, int &initial_setting_flag) {
    unsigned char metaType = data[pos++]; // Read meta event type
    int length = static_cast<int>(data[pos++]); // Read meta event length (as int)

    size_t startPos = pos; // Store initial position to ensure correct length handling

    if (metaType == 0x21 && length == 1) { // MIDI Port
        initial_setting_flag = 1;
    }
    else if (metaType == 0x58 && length == 4) { // Time Signature
        unsigned char numerator = data[pos];      
        unsigned char denominator = 1 << data[pos + 1]; 
        std::cout << "  - Time Signature: " << (int)numerator << "/" << (int)denominator << "\n";
    } 
    else if (metaType == 0x51 && length == 3) { // Set Tempo
        int tempo = ((data[pos] & 0xFF) << 16) | 
                            ((data[pos + 1] & 0xFF) << 8) | 
                            (data[pos + 2] & 0xFF);
        
        int bpm = 60000000 / tempo;
        std::cout << "  - Tempo Change: " << bpm << " BPM\n";
    }
    else if (metaType == 0x2F) { // End of Track
        std::cout << "  - End of Track reached\n";
    }

    // Ensure we move `pos` exactly `length` bytes forward
    pos = startPos + length;
}


void handleChannel10(const std::vector<unsigned char>& data, size_t& pos, unsigned char eventType) {
    unsigned char channel = eventType & 0x0F; // Extract channel 
    unsigned char control = data[pos++];      // Read control type

    if (eventType == 0xB9) { // Control Change (0xB9)
        pos++;
    } 
}

// Process Note On Events (Using Switch Case)
void handleNoteOn(const std::vector<unsigned char>& data, size_t& pos, double &note_on_time, int tpqn, const std::string& midiFilePath) {
    if (pos + 2 > data.size()) {
        std::cerr << "[Error] Not enough data for Note On event.\n";
        return;
    }

    unsigned char drumNote = data[pos++];  // Drum (Note Number)
    unsigned char velocity = data[pos++];  // Velocity (Intensity)

    std::string drumName;

    // Switch-case to map drum notes
    switch ((int)drumNote) {
        case 36: drumName = "Bass Drum 1"; break;
        case 41: drumName = "Low Floor Tom"; break;
        case 38: drumName = "Acoustic Snare"; break;
        case 45: drumName = "Low Tom"; break;
        case 47: drumName = "Low Mid Tom"; break;
        case 48: drumName = "Low Mid Tom"; break;
        case 50: drumName = "Low Mid Tom"; break;
        case 42: drumName = "Closed Hi-Hat"; break;
        case 46: drumName = "Open Hi-Hat"; break;
        case 49: drumName = "Crash Cymbal 1"; break;
        case 51: drumName = "Ride Cymbal 1"; break;
        case 57: drumName = "Crash Cymbal 2"; break;
        default: drumName = "Unknown Drum"; break;
    }

    //tick per real time
    // (ticks * 60000)/tpqn *bpm
    if(velocity > 0)
    {
        note_on_time = ((note_on_time*60000)/(100 * tpqn))/1000;
        std::cout << note_on_time <<"s \t"<< "Hit Drum: " << drumName << " -> "  << (int)drumNote<< "\n";
        save_to_csv(midiFilePath, note_on_time, (int)drumNote);
    }

    //save to csv

}
void save_to_csv(const std::string& outputCsvPath, double &note_on_time, int drumNote) {
    std::ofstream file(outputCsvPath, std::ios::app);
    if (!file) {
        std::cerr << "Failed to open CSV file: " << outputCsvPath << std::endl;
        return;
    }

    int mappedDrumNote;
    std::string drumName;

    switch (drumNote) {
        case 38: mappedDrumNote = 1; break;
        case 41: mappedDrumNote = 2; break;
        case 45: mappedDrumNote = 3; break;
        case 47: mappedDrumNote = 4; break;
        case 48: mappedDrumNote = 4; break;
        case 50: mappedDrumNote = 4; break;
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



// Read MIDI file into buffer
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

int main() {
    std::string midiNameOnly;  // ex: "input1"
    size_t pos;
    unsigned char runningStatus;
    int initial_setting_flag = 0;
    double note_on_time = 0;

    std::cout << "Enter MIDI file name (without .mid): ";
    std::cin >> midiNameOnly;

    // 경로 설정
    std::string inputPath  = "/home/taehwang/basic-algo-lecture-master/drum_roobt/midifile/midbox/" + midiNameOnly + ".mid";
    std::string outputPath = "/home/taehwang/basic-algo-lecture-master/drum_roobt/midifile/mid2midcode/" + midiNameOnly + "_mc.csv";

    // MIDI 읽기
    std::vector<unsigned char> midiData;
    if (!readMidiFile(inputPath, midiData)) {
        return 1;
    }

    pos = 14; // header chunk (MThd 14 bytes)

    int tpqn = (midiData[12] << 8) | midiData[13];
    std::cout << "Time Division (TPQN): " << tpqn << " ticks per quarter note\n";
    
    while (pos + 8 <= midiData.size()) {
        // MTrk 시그니처 확인
        if (!(midiData[pos] == 'M' && midiData[pos+1] == 'T' &&
              midiData[pos+2] == 'r' && midiData[pos+3] == 'k')) {
            std::cerr << "🚫 MTrk expected at pos " << pos << "\n";
            break;
        }
    
        size_t trackLength = (midiData[pos+4] << 24) |
                             (midiData[pos+5] << 16) |
                             (midiData[pos+6] << 8) |
                             midiData[pos+7];
    
        pos += 8;
        size_t trackEnd = pos + trackLength;
    
        std::cout << "🎵 Reading MTrk (length=" << trackLength << ") from pos " << pos << " to " << trackEnd << "\n";
    
        double note_on_time = 0;
    
        while (pos < trackEnd) {
            size_t delta = readTime(midiData, pos);
            note_on_time += delta;
            analyzeMidiEvent(midiData, pos, runningStatus, initial_setting_flag, note_on_time, tpqn, outputPath);
        }
    
        pos = trackEnd;
    }
    

    std::cout << "코드 끗." << "\n";
    return 0;
}
