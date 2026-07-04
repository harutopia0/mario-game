#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "engine/audio/miniaudio.h"
#include <map>
#include <string>

class AudioManager
{
  public:
    // Lấy instance duy nhất của AudioManager (Singleton)
    static AudioManager *GetInstance();

    // Dừng nhạc và giải phóng hệ thống engine âm thanh
    void CleanUp();

    // Lưu đường dẫn file âm thanh kèm theo ID định danh (VD: "jump" -> "sounds/jump.wav")
    bool LoadSound(std::string id, std::string source);

    // Phát nhạc nền (BGM), hỗ trợ lặp lại, tự động tắt nhạc cũ trước khi phát
    void PlayMusic(std::string id, bool loop = true);

    // Phát hiệu ứng âm thanh (SFX) một lần, tự động hủy khi phát xong
    void PlaySFX(std::string id);

    // Tạm dừng nhạc nền
    void PauseMusic();

    // Tiếp tục phát nhạc nền
    void ResumeMusic();

    // Phát nhạc sự kiện (chèn ngang nhạc nền)
    void PlayEventMusic(std::string id, bool loop = true);

    // Dừng nhạc sự kiện
    void StopEventMusic();

    // Dừng và giải phóng nhạc nền đang phát hiện tại
    void StopMusic();

  private:
    // Khởi tạo engine âm thanh
    AudioManager();
    ~AudioManager();

    static AudioManager *s_Instance;

    ma_engine m_Engine;
    ma_sound m_CurrentMusic; // Dùng riêng để điều khiển nhạc nền
    bool m_IsMusicPlaying;
    std::string m_CurrentMusicId; // ID của bài nhạc nền đang phát

    ma_sound m_EventMusic; // Nhạc sự kiện (Star)
    bool m_IsEventMusicPlaying;

    // Danh sách lưu trữ cặp <ID, Đường dẫn file>
    std::map<std::string, std::string> m_SoundMap;
};

#endif // AUDIO_MANAGER_H