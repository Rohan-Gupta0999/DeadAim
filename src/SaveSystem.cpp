#include "SaveSystem.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <utility>

namespace deadaim {

SaveSystem::SaveSystem(std::string filePath)
    : m_filePath(std::move(filePath))
{
}

void SaveSystem::load() {
    std::ifstream file(m_filePath);
    if (!file.is_open()) {
        std::cerr << "[SaveSystem] No save file yet -- starting fresh.\n";
        return;
    }

    try {
        nlohmann::json data;
        file >> data;
        m_highScore = data.value("high_score", 0);

        if (data.contains("settings")) {
            const auto& s = data["settings"];
            m_settings.musicVolume = s.value("music_volume", m_settings.musicVolume);
            m_settings.sfxVolume = s.value("sfx_volume", m_settings.sfxVolume);
            m_settings.sensitivity = s.value("sensitivity", m_settings.sensitivity);
        }

        std::cerr << "[SaveSystem] Loaded high score: " << m_highScore << "\n";
    } catch (const nlohmann::json::exception&) {
        std::cerr << "[SaveSystem] Save file unreadable -- ignoring it.\n";
        m_highScore = 0;
        m_settings = Settings{};
    }
}

const Settings& SaveSystem::getSettings() const {
    return m_settings;
}

void SaveSystem::updateSettings(const Settings& settings) {
    m_settings = settings;
    save();
}

bool SaveSystem::submitScore(int score) {
    if (score <= m_highScore) {
        return false; // nothing to write
    }

    m_highScore = score;
    save();
    return true;
}

int SaveSystem::getHighScore() const {
    return m_highScore;
}

void SaveSystem::save() const {
    nlohmann::json data;
    data["high_score"] = m_highScore;
    data["settings"] = {
        {"music_volume", m_settings.musicVolume},
        {"sfx_volume", m_settings.sfxVolume},
        {"sensitivity", m_settings.sensitivity}
    };

    std::ofstream file(m_filePath);
    if (!file.is_open()) {
        std::cerr << "[SaveSystem] Could not write save file: " << m_filePath << "\n";
        return;
    }

    file << data.dump(2);
}

} 