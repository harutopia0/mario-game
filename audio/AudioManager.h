#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "miniaudio.h"
#include <string>
#include <map>

class AudioManager {
public:
    static AudioManager* GetInstance();
    void CleanUp();

    // Lưu đường dẫn file âm thanh
    bool LoadSound(std::string id, std::string source);

    // Phát âm thanh
    void PlayMusic(std::string id, bool loop = true);
    void PlaySFX(std::string id);
    void StopMusic();

private:
    AudioManager();
    ~AudioManager();

    static AudioManager* s_Instance;

    ma_engine m_Engine;
    ma_sound m_CurrentMusic; // Dùng riêng để điều khiển nhạc nền
    bool m_IsMusicPlaying;

    std::map<std::string, std::string> m_SoundMap;
};

#endif // AUDIO_MANAGER_H