#pragma once

#include <SFML/Audio.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace deadaim {


enum class SoundId {
    GunFire,
    BowFire,
    FireballFire,
    Explosion,
    ZombieDeath,
    PlayerHurt,
    MenuClick,
    GameOver,
    NewHighScore
};


class AudioSystem {
public:
    AudioSystem();

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    void play(SoundId id);

    void startMusic();   // no-op if already playing
    void stopMusic();

    void setSfxVolume(float volume);    // 0..100
    void setMusicVolume(float volume);  // 0..100
    void pauseMusic();
    void resumeMusic();
private:
    void loadSound(SoundId id, const std::string& path);

    std::unordered_map<int, sf::SoundBuffer> m_buffers; 
    std::vector<std::unique_ptr<sf::Sound>> m_soundPool;
    sf::Music m_music;
    bool m_musicReady = false;

    float m_sfxVolume = 70.f;
    float m_musicVolume = 35.f; 

    static constexpr std::size_t kPoolSize = 16;
};

} 