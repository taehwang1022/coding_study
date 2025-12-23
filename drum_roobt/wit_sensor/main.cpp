#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>   // 시간 측정을 위해
#include <iomanip>  // 시간 형식(put_time)
#include <sstream>  // 문자열 스트림
#include <thread>   // sleep_for

// --- Linux 시리얼 통신 헤더 ---
#include <fcntl.h>   // File control definitions
#include <termios.h> // POSIX terminal control definitions
#include <unistd.h>  // UNIX standard function definitions
#include <errno.h>   // Error number definitions

// ==========================================================
//                 ⚠️ 사용자 설정 변수 ⚠️
// ==========================================================

// 1. 실제 연결된 시리얼 포트 경로로 수정하세요.
//    (터미널에서 'ls /dev/ttyUSB*' 또는 'dmesg | grep tty'로 확인)
const std::string SERIAL_PORT = "/dev/ttyUSB0"; 

// 2. 센서에 설정된 실제 측정 범위로 수정하세요. (데이터 변환에 필수)
//    (WitMotion 프로그램으로 확인 가능, 모를 경우 기본값 사용)
const float ACC_RANGE_G = 16.0;      // 가속도 측정 범위 (기본값 예: 16g)
const float GYRO_RANGE_DPS = 2000.0; // 각속도 측정 범위 (기본값 예: 2000°/s)

// 3. 저장할 CSV 파일 이름
const std::string CSV_FILENAME = "sensor_log.csv";
// ==========================================================


// WitMotion 프로토콜 상수
const unsigned char HEADER = 0x55;
const unsigned char ACC_TAG = 0x51;  // 가속도
const unsigned char GYRO_TAG = 0x52; // 각속도

// 센서 데이터 저장을 위한 구조체
struct SensorData {
    std::string timestamp;
    char dataType = 0; // 'A' (Accel) 또는 'G' (Gyro)
    float x = 0.0, y = 0.0, z = 0.0;
};

// 현재 시간을 밀리초까지 포맷팅하여 반환
std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm_buf;
    // 스레드 안전한 localtime_r 사용
    localtime_r(&t, &tm_buf);

    std::stringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setw(3) << std::setfill('0') << ms.count();
    return ss.str();
}

// 16비트 부호 있는 정수(little-endian)로 변환
short parseShort(unsigned char low, unsigned char high) {
    return (short)((high << 8) | low);
}

// 시리얼 포트 열기 및 설정
int configureSerialPort(const std::string& port) {
    int fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        std::cerr << "오류: 시리얼 포트 열기 실패 (" << port << ")" << std::endl;
        return -1;
    }

    // 포트가 블로킹 모드(데이터 대기)로 동작하도록 FNDELAY 플래그를 해제
    fcntl(fd, F_SETFL, 0);

    termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        std::cerr << "오류: tcgetattr" << std::endl;
        close(fd);
        return -1;
    }

    // --- Baud Rate 설정 (115200) ---
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // --- 필수 플래그 설정 ---
    
    // CREAD: 수신 활성화
    // CLOCAL: 모뎀 제어 라인 무시 (필수!)
    tty.c_cflag |= (CLOCAL | CREAD);
    
    // 8N1 설정
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    
    // 하드웨어/소프트웨어 흐름 제어(Flow Control) 비활성화 (매우 중요!)
    tty.c_cflag &= ~CRTSCTS; // 하드웨어 흐름 제어
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // 소프트웨어 흐름 제어

    // Raw 모드 설정 (가공되지 않은 데이터 수신)
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_oflag &= ~OPOST;

    // --- Read 타임아웃 설정 (중요) ---
    // VMIN = 0, VTIME = 5 : 0.5초 타임아웃을 가진 non-blocking read
    // VMIN > 0, VTIME = 0 : VMIN 바이트가 수신될 때까지 무한 대기 (blocking)
    
    // [변경] VMIN = 1, VTIME = 0 : 최소 1바이트가 올 때까지 '무한 대기(Blocking)'
    // 이 설정은 데이터가 안 올 경우 프로그램이 여기서 멈춰있지만,
    // 데이터가 오기만 한다면 확실하게 수신을 시작합니다.
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    // 설정 적용
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        std::cerr << "오류: tcsetattr" << std::endl;
        close(fd);
        return -1;
    }
    
    // 시리얼 버퍼 비우기 (이전 쓰레기 데이터 제거)
    tcflush(fd, TCIOFLUSH);

    std::cout << "[설정 완료] 포트가 Blocking 모드로 대기합니다." << std::endl;
    return fd;
}

// 패킷 파싱 및 데이터 변환
bool parsePacket(const std::vector<unsigned char>& packet, SensorData& data) {
    // 1. 체크섬 검증
    unsigned char checksum = 0;
    for (int i = 0; i < 10; ++i) {
        checksum += packet[i];
    }
    if (checksum != packet[10]) {
        std::cerr << "경고: 체크섬 오류" << std::endl;
        return false;
    }

    // 2. 데이터 태그 확인
    unsigned char tag = packet[1];
    float scale;

    if (tag == ACC_TAG) {
        data.dataType = 'A'; // 가속도
        scale = ACC_RANGE_G / 32768.0f;
    } else if (tag == GYRO_TAG) {
        data.dataType = 'G'; // 각속도
        scale = GYRO_RANGE_DPS / 32768.0f;
    } else {
        return false; // 우리가 찾는 데이터가 아님 (예: 0x53 각도)
    }

    // 3. 데이터 파싱 (X, Y, Z 축)
    data.x = parseShort(packet[2], packet[3]) * scale;
    data.y = parseShort(packet[4], packet[5]) * scale;
    data.z = parseShort(packet[6], packet[7]) * scale;
    // (packet[8], packet[9]는 온도 또는 기타 데이터일 수 있으나 여기서는 무시)

    return true;
}


int main() {
    int serial_fd = configureSerialPort(SERIAL_PORT);
    if (serial_fd < 0) {
        return 1;
    }

    std::ofstream csvFile;
    // 파일 존재 여부 및 크기 확인
    bool file_exists = std::ifstream(CSV_FILENAME).good();
    
    // 파일 열기 (기존 내용에 이어 쓰기 모드)
    csvFile.open(CSV_FILENAME, std::ios::out | std::ios::app);
    if (!csvFile.is_open()) {
        std::cerr << "오류: CSV 파일 열기 실패 (" << CSV_FILENAME << ")" << std::endl;
        close(serial_fd);
        return 1;
    }

    // --- 🚨 디버깅된 헤더 작성 로직 🚨 ---
    // 파일이 새로 생성되었거나 비어있는 경우에만 헤더 작성
    if (!file_exists || csvFile.tellp() == 0) {
        csvFile << "Timestamp,Type,X,Y,Z\n";
        csvFile.flush(); // 즉시 파일에 쓰도록 버퍼 비우기
        std::cout << "✅ CSV 헤더 작성 완료." << std::endl;
    }
    // ----------------------------------------

    std::cout << "센서 데이터 수신 및 로깅 시작... (Ctrl+C로 종료)" << std::endl;
    std::cout << "포트: " << SERIAL_PORT << ", 파일: " << CSV_FILENAME << std::endl;

    // ... (이후 시리얼 통신 및 데이터 처리 로직은 동일)

    std::vector<unsigned char> buffer;
    unsigned char byte_buffer[1];

    while (true) {
        // 1. 시리얼 포트에서 1바이트 읽기
        int n = read(serial_fd, byte_buffer, 1);

        if (n < 0) {
            // Error handling (N < 0)
            if (errno == EINTR) continue; // signal interrupt
            std::cerr << "오류: 시리얼 읽기 오류" << std::endl;
            break;
        }

        if (n == 0) {
            // 데이터 없음 (Blocking 모드이므로 거의 발생하지 않음)
            continue;
        }

        // 2. 패킷 시작(HEADER) 찾기
        if (buffer.empty() && byte_buffer[0] != HEADER) {
            continue; 
        }

        buffer.push_back(byte_buffer[0]);

        // 3. 패킷 11바이트 수집
        if (buffer.size() == 11) {
            if (buffer[0] == HEADER) {
                SensorData data;
                if (parsePacket(buffer, data)) {
                    // 4. 유효한 데이터(가속도/각속도)인 경우
                    data.timestamp = getTimestamp();

                    // CSV 파일에 쓰기
                    csvFile << data.timestamp << ","
                            << data.dataType << ","
                            << data.x << ","
                            << data.y << ","
                            << data.z << "\n";
                    
                    // 데이터 버퍼 비우기 (데이터 손실 방지)
                    csvFile.flush();
                    
                    // (선택 사항) 터미널에도 출력
                    std::cout << data.timestamp << " | " << data.dataType
                              << " | X: " << std::setw(8) << data.x
                              << " | Y: " << std::setw(8) << data.y
                              << " | Z: " << std::setw(8) << data.z << std::endl;
                }
            }
            // 패킷 처리가 끝났으므로 버퍼 비우기
            buffer.clear();
        }
    }

    std::cout << "로깅 종료." << std::endl;
    csvFile.close();
    close(serial_fd);
    return 0;
}