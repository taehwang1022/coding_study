#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>


// Function Declarations
bool readMidiHeader(const std::vector<unsigned char>& data);
size_t readTime(const std::vector<unsigned char>& data, size_t& pos);
void analyzeMidiEvent(const std::vector<unsigned char>& data, size_t& pos, unsigned char& runningStatus);
void handleMetaEvent(const std::vector<unsigned char>& data, size_t& pos, int& tempo);
void handleChannel10(const std::vector<unsigned char>& data, size_t& pos, unsigned char eventType);
void handleNoteOn(const std::vector<unsigned char>& data, size_t& pos);
bool readMidiFile(const std::string& filename, std::vector<unsigned char>& buffer);

// Read the MIDI header chunk
bool readMidiHeader(const std::vector<unsigned char>& data) {
    if (data.size() < 14) {
        std::cerr << "File size is too small to read the MIDI header.\n";
        return false;
    }

    // Check if the header contains "MThd" (4D 54 68 64)
    if (data[0] != 0x4D || data[1] != 0x54 || data[2] != 0x68 || data[3] != 0x64) {
        std::cerr << "Invalid MIDI file (MThd not found).\n";
        return false;
    }

    // Header chunk size (always 6 bytes)
    int headerChunkSize = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];

    // MIDI format (2 bytes)
    int format = (data[8] << 8) | data[9];

    // Number of tracks (2 bytes)
    int trackCount = (data[10] << 8) | data[11];

    // Time division (2 bytes)
    int division = (data[12] << 8) | data[13];

    std::cout << "MIDI File Analysis:\n";
    std::cout << "Header Chunk Size: " << headerChunkSize << " bytes\n";
    std::cout << "Format: " << format << " (";
    if (format == 0) std::cout << "Single Track";
    else if (format == 1) std::cout << "Multi Track";
    else if (format == 2) std::cout << "Independent Multi Track";
    std::cout << ")\n";
    std::cout << "Number of Tracks: " << trackCount << std::endl;
    std::cout << "Time Division (TPQN): " << division << " ticks per quarter note\n";

    // Analyze the first track chunk
    size_t pos = 14;
    if (pos + 8 <= data.size()) {
        if (data[pos] == 0x4D && data[pos + 1] == 0x54 &&
            data[pos + 2] == 0x72 && data[pos + 3] == 0x6B) {
            
            int trackChunkSize = (data[pos + 4] << 24) | (data[pos + 5] << 16) |
                                 (data[pos + 6] << 8) | data[pos + 7];

            std::cout << "First Track Chunk Found:\n";
            std::cout << "Track Chunk Identifier: MTrk (4D 54 72 6B)\n";
            std::cout << "Track Chunk Size: " << trackChunkSize << " bytes\n";
        } else {
            std::cerr << "Valid track chunk not found.\n";
            return false;
        }
    } else {
        std::cerr << "File size is too small to read the track chunk.\n";
        return false;
    }

    return true;
}

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
void analyzeMidiEvent(const std::vector<unsigned char>& data, size_t& pos, unsigned char& runningStatus, int& tempo) {
    if (pos >= data.size()) return;

    unsigned char eventType = data[pos];

    // Update Running Status only for valid event types
    if (eventType == 0xFF || eventType == 0xB9 || eventType == 0xC9 || eventType == 0x99) {  
        runningStatus = eventType;
        pos++; // Consume event byte
    } else {
        // Retain Running Status if event type is missing
        std::cout << "[Running Status] Previous event retained: " << std::hex << (int)runningStatus << "\n";
        eventType = runningStatus;
    }

    // Handle Meta Event
    if (eventType == 0xFF) { 
        handleMetaEvent(data, pos, tempo);
    }
    // Handle Channel 10 events (0xB9: Control Change, 0xC9: Program Change)
    else if (eventType == 0xB9 || eventType == 0xC9) { 
        handleChannel10(data, pos, eventType);
    }
    // Handle Note On events (0x99)
    else if (eventType  == 0x99) { 
        handleNoteOn(data, pos);
    }
}
// Process Meta Events (Essential Only)
void handleMetaEvent(const std::vector<unsigned char>& data, size_t& pos, int& tempo) {
    unsigned char metaType = data[pos++]; // Read meta event type
    int length = static_cast<int>(data[pos++]); // Read meta event length (as int)

    std::string metaEventName;

    // Essential Meta Event Mapping
    switch (metaType) {
        case 0x03: metaEventName = "Track Name"; break;
        case 0x21: metaEventName = "MIDI Port"; break;
        case 0x51: metaEventName = "Set Tempo"; break;
        case 0x58: metaEventName = "Time Signature"; break;
        case 0x59: metaEventName = "Key Signature"; break;
        case 0x2F: metaEventName = "End of Track"; break;
        default: metaEventName = "Unknown Meta Event"; break;
    }

    std::cout << "[Meta Event] " << metaEventName 
              << " (Type: " << (int)metaType 
              << ", Length: " << length << ")\n";

    size_t startPos = pos; // Store initial position to ensure correct length handling

    if (metaType == 0x03) { // Track Name
        std::vector<unsigned char> utf8String(data.begin() + pos, data.begin() + pos + length);
        std::string trackName(utf8String.begin(), utf8String.end());
        std::cout << "  - Track Name: " << trackName << "\n";
    } 
    else if (metaType == 0x21 && length == 1) { // MIDI Port
        unsigned char port = data[pos];
        std::cout << "  - MIDI Port: " << (int)port << "\n";
    }
    else if (metaType == 0x58 && length == 4) { // Time Signature
        unsigned char numerator = data[pos];      
        unsigned char denominator = 1 << data[pos + 1]; 
        std::cout << "  - Time Signature: " << (int)numerator << "/" << (int)denominator << "\n";
    } 
    else if (metaType == 0x59 && length == 2) { // Key Signature
        int key = (signed char)data[pos];  
        std::string scale = (data[pos + 1] == 0) ? "Major" : "Minor";
        std::cout << "  - Key Signature: " << key << " (" << scale << ")\n";
    } 
    else if (metaType == 0x51 && length == 3) { // Set Tempo
        unsigned int tempo = ((data[pos] & 0xFF) << 16) | 
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

// Process Channel 10 Events
void handleChannel10(const std::vector<unsigned char>& data, size_t& pos, unsigned char eventType) {
    unsigned char channel = eventType & 0x0F; // Extract channel 
    unsigned char control = data[pos++];      // Read control type

    if (eventType == 0xB9) { // Control Change (0xB9)
        unsigned char value = data[pos++]; // Read value

        if (control == 0x79) {
            std::cout << "[Channel "<<channel <<"] Reset All Controllers (Value: " << (int)value << ")\n";
        } 
        else if (control == 0x07) {
            std::cout << "[Channel "<<channel <<"] Volume Control: " << (int)value << "\n";
        } 
        else if (control == 0x0A) {
            std::cout << "[Channel "<<channel <<"] Pan Position: " << (int)value << "\n";
        } 
        else if (control == 0x5B) {  
            std::cout << "[Channel "<<channel <<"] Reverb: " << (int)value << "\n";
        } 
        else if (control == 0x5D) {  
            std::cout << "[Channel "<<channel <<"] Chorus: " << (int)value << "\n";
        } 
        else {
            std::cout << "[Channel "<<channel <<"] Unknown Control Change (Control: " << (int)control 
                      << ", Value: " << (int)value << ")\n";
        }
    } 
    else if (eventType == 0xC9) { // Program Change (0xC9)
        unsigned char program = data[pos];
        std::cout << "[Channel 10] Program Change: " << (int)program << "\n";
    } 
    else {
        std::cout << "[Channel 10] Unknown Event (Type: " << (int)eventType << ")\n";
    }
}

// Process Note On Events (Using Switch Case)
void handleNoteOn(const std::vector<unsigned char>& data, size_t& pos) {
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
        case 42: drumName = "Closed Hi-Hat"; break;
        case 46: drumName = "Open Hi-Hat"; break;
        case 49: drumName = "Crash Cymbal 1"; break;
        default: drumName = "Unknown Drum"; break;
    }

    std::cout << "[Note On] Drum: " << drumName 
              << " (Note: " << (int)drumNote << "), Velocity: " << (int)velocity << "\n";
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
    std::string midiFilePath;
    size_t pos;
    unsigned char runningStatus;

    std::cout << "Enter MIDI file name: ";
    std::cin >> midiFilePath;

    std::vector<unsigned char> midiData;
    if (readMidiFile(midiFilePath, midiData)) {
        if (readMidiHeader(midiData)) {
            std::cout << "\nMIDI Header and Track Chunk Analysis Completed!\n";
            pos = 22;
        }
    } else {
        std::cerr << "Failed to read MIDI file.\n";
    }

    int tempo = 60000000 / 100; // 100 BPM → 600,000 microseconds per quarter note
    int tpqn = 480; // Ticks per quarter note

    while (pos < midiData.size()) {
        size_t time = readTime(midiData, pos);

        // ✅ Delta Time을 초 단위로 변환
        double seconds = (static_cast<double>(time) * tempo) / (tpqn * 1000000.0);

        // Print a separator box for clarity
        std::cout << "\n====================================\n";
        std::cout << "  MIDI Event Read\n";
        std::cout << std::dec; // 10진수 출력 설정
        std::cout << "  Position: " << pos << "\n";
        std::cout << "  Delta Time: " << static_cast<int>(time) << " ticks\n";
        std::cout << "  Time (Seconds): " << seconds << " s\n"; // ✅ 초 단위 변환 추가
        std::cout << "------------------------------------\n";

        analyzeMidiEvent(midiData, pos, runningStatus, tempo); // ✅ tempo 전달

        std::cout << "====================================\n";
    }

    return 0;
}
