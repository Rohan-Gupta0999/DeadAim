#include "AudioSystem.h"
#include <iostream>

namespace deadaim {

AudioSystem::AudioSystem() {
    loadSound(SoundId::GunFire,      "assets/audio/gun.wav");
    loadSound(SoundId::BowFire,      "assets/audio/bow.wav");
    loadSound(SoundId::FireballFire, "assets/audio/fireball.wav");
    loadSound(SoundId::Explosion,    "assets/audio/explosion.wav");
    loadSound(SoundId::ZombieDeath,  "assets/audio/zombie_death.wav");
    loadSound(SoundId::PlayerHurt,   "assets/audio/player_hurt.wav");
    loadSound(SoundId::MenuClick,    "assets/audio/menu_click.wav");
    loadSound(SoundId::GameOver,     "assets/audio/game_over.wav");
    loadSound(SoundId::NewHighScore, "assets/audio/new_high_score.wav");

    // sf::Sound needs a buffer at construction in SFML 3, so the pool is
    // seeded from whichever buffer loaded first -- play() swaps in the
    // right one before each use. If nothing loaded at all, the pool stays
    // empty and every play() call is a harmless no-op.
    if (!m_buffers.empty()) {
        const sf::SoundBuffer& seed = m_buffers.begin()->second;
        m_soundPool.reserve(kPoolSize);
        for (std::size_t i = 0; i < kPoolSize; ++i) {
            m_soundPool.push_back(std::make_unique<sf::Sound>(seed));
        }
    } else {
        std::cerr << "[AudioSystem] No sound effects loaded -- running silent.\n";
    }

    if (m_music.openFromFile("assets/music/background.ogg")) {
        m_music.setLooping(true);
        m_music.setVolume(m_musicVolume);
        m_musicReady = true;
    } else {
        std::cerr << "[AudioSystem] No background music found -- skipping.\n";
    }
}

void AudioSystem::loadSound(SoundId id, const std::string& path) {
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(path)) {
        std::cerr << "[AudioSystem] Missing sound: " << path << "\n";
        return; // absent from the map; play() will silently skip it
    }
    m_buffers.emplace(static_cast<int>(id), std::move(buffer));
}

void AudioSystem::play(SoundId id) {
    auto it = m_buffers.find(static_cast<int>(id));
    if (it == m_buffers.end()) {
        return; // never loaded -- stay silent rather than complain every frame
    }

    for (auto& sound : m_soundPool) {
        if (sound->getStatus() != sf::SoundSource::Status::Playing) {
            sound->setBuffer(it->second);
            sound->setVolume(m_sfxVolume);
            sound->play();
            return;
        }
    }
    // Pool exhausted: drop this sound rather than cut off one already
    // playing. A missed sound during chaos is imperceptible; a chopped
    // one is obvious.
}

void AudioSystem::startMusic() {
    if (m_musicReady && m_music.getStatus() != sf::SoundSource::Status::Playing) {
        m_music.play();
    }
}

void AudioSystem::stopMusic() {
    if (m_musicReady) {
        m_music.stop();
    }
}

void AudioSystem::setSfxVolume(float volume) {
    m_sfxVolume = volume;
}

void AudioSystem::setMusicVolume(float volume) {
    m_musicVolume = volume;
    if (m_musicReady) {
        m_music.setVolume(volume);
    }
}

} // namespace deadaim