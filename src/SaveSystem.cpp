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
        // First run: expected, not a failure.
        std::cerr << "[SaveSystem] No save file yet -- starting fresh.\n";
        return;
    }

    try {
        nlohmann::json data;
        file >> data;
        m_highScore = data.value("high_score", 0);
        std::cerr << "[SaveSystem] Loaded high score: " << m_highScore << "\n";
    } catch (const nlohmann::json::exception&) {
        // A corrupt file shouldn't cost anyone their game session.
        std::cerr << "[SaveSystem] Save file unreadable -- ignoring it.\n";
        m_highScore = 0;
    }
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

    std::ofstream file(m_filePath);
    if (!file.is_open()) {
        std::cerr << "[SaveSystem] Could not write save file: " << m_filePath << "\n";
        return;
    }

    file << data.dump(2); // indented: this file is meant to be readable during testing
}

} // namespace deadaim