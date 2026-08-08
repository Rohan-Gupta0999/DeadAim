#pragma once

#include <SFML/Audio.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace deadaim {

// Every sound the game can play. An enum rather than string lookups:
// a typo becomes a compile error instead of a silent no-op.
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

// Purpose: the single owner of all audio playback -- sound effects and
// background music.
//
// Responsibilities: load every sound once at startup; play a sound
// effect on a free pooled voice; stream and loop background music;
// expose volume controls.
//
// Dependencies: SFML (Audio module).
//
// Design note: sounds load once at construction rather than on demand.
// On-demand loading would retry a missing file on every gunshot and
// flood the console; loading up front makes a missing asset a known
// startup state, logged once and then silently absent.
//
// Lifetime rule: sf::Sound references its buffer rather than copying it,
// so m_buffers MUST stay declared before m_soundPool -- members destruct
// in reverse declaration order.
//
// Future extension points: setSfxVolume/setMusicVolume already exist for
// the Settings system to drive. Separate menu and gameplay tracks would
// mean a second sf::Music member.
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

private:
    void loadSound(SoundId id, const std::string& path);

    std::unordered_map<int, sf::SoundBuffer> m_buffers; // declared FIRST -- see lifetime rule
    std::vector<std::unique_ptr<sf::Sound>> m_soundPool;
    sf::Music m_music;
    bool m_musicReady = false;

    float m_sfxVolume = 70.f;
    float m_musicVolume = 35.f; // deliberately quieter: music sits under the action

    static constexpr std::size_t kPoolSize = 16;
};

} // namespace deadaim