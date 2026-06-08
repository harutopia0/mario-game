#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "miniaudio.h"
#include <string>
#include <map>

class AudioManager {
public:
    // Lấy instance duy nhất của AudioManager (Singleton)
    static AudioManager* GetInstance();

    // Dừng nhạc và giải phóng hệ thống engine âm thanh
    void CleanUp();

    // Lưu đường dẫn file âm thanh kèm theo ID định danh (VD: "jump" -> "sounds/jump.wav")
    bool LoadSound(std::string id, std::string source);

    // Phát nhạc nền (BGM), hỗ trợ lặp lại, tự động tắt nhạc cũ trước khi phát
    void PlayMusic(std::string id, bool loop = true);

    // Phát hiệu ứng âm thanh (SFX) một lần, tự động hủy khi phát xong
    void PlaySFX(std::string id);

    // Dừng và giải phóng nhạc nền đang phát hiện tại
    void StopMusic();

private:
    // Khởi tạo engine âm thanh
    AudioManager();
    ~AudioManager();

    static AudioManager* s_Instance;

    ma_engine m_Engine;
    ma_sound m_CurrentMusic; // Dùng riêng để điều khiển nhạc nền
    bool m_IsMusicPlaying;

    // Danh sách lưu trữ cặp <ID, Đường dẫn file>
    std::map<std::string, std::string> m_SoundMap;
};

#endif // AUDIO_MANAGER_H