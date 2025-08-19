#include <SFML/Audio.hpp>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::string fileName;
    std::cout << "재생할 음악 파일명을 입력하세요 (예: VLV_short.wav): ";
    std::cin >> fileName;

    std::string fullPath = "/home/taehwang/basic-algo-lecture-master/drum_roobt/" + fileName;

    sf::Music music;
    if (!music.openFromFile(fullPath)) {
        std::cerr << "🎵 음악 파일 열기 실패: " << fullPath << "\n";
        return 1;
    }

    music.setVolume(100); // 볼륨 100%

    std::cout << "🎵 음악 준비 완료. 3초 뒤에 재생됩니다...\n";

    auto syncTime = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (std::chrono::steady_clock::now() < syncTime) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    music.play();
    std::cout << "🎵 음악 재생 시작!\n";

    while (music.getStatus() == sf::Music::Playing) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "🎵 음악 재생 종료\n";
    return 0;
}
