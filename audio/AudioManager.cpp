#include "AudioManager.h"
#include <iostream>

// Khởi tạo mã nguồn miniaudio (Chỉ gọi #define này 1 lần duy nhất trong toàn dự án)
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

AudioManager* AudioManager::s_Instance = nullptr;

AudioManager::AudioManager() : m_IsMusicPlaying(false) {
    if (ma_engine_init(NULL, &m_Engine) != MA_SUCCESS) {
        std::cerr << "Lỗi khởi tạo miniaudio!\n";
    }
}

AudioManager::~AudioManager() {
    CleanUp();
}

AudioManager* AudioManager::GetInstance() {
    if (s_Instance == nullptr) {
        s_Instance = new AudioManager();
    }
    return s_Instance;
}

void AudioManager::CleanUp() {
    StopMusic();
    ma_engine_uninit(&m_Engine);
}

bool AudioManager::LoadSound(std::string id, std::string source) {
    m_SoundMap[id] = source;
    return true;
}

void AudioManager::PlaySFX(std::string id) {
    if (m_SoundMap.find(id) != m_SoundMap.end()) {
        // Phát SFX tự động tự hủy khi chạy xong
        ma_engine_play_sound(&m_Engine, m_SoundMap[id].c_str(), NULL);
    }
}

void AudioManager::PlayMusic(std::string id, bool loop) {
    if (m_SoundMap.find(id) == m_SoundMap.end()) return;

    StopMusic(); // Tắt nhạc cũ trước khi phát nhạc mới

    ma_result result = ma_sound_init_from_file(&m_Engine, m_SoundMap[id].c_str(), 0, NULL, NULL, &m_CurrentMusic);
    if (result == MA_SUCCESS) {
        ma_sound_set_looping(&m_CurrentMusic, loop);
        ma_sound_start(&m_CurrentMusic);
        m_IsMusicPlaying = true;
    }
}

void AudioManager::StopMusic() {
    if (m_IsMusicPlaying) {
        ma_sound_stop(&m_CurrentMusic);
        ma_sound_uninit(&m_CurrentMusic); // Giải phóng bộ nhớ nhạc nền
        m_IsMusicPlaying = false;
    }
}