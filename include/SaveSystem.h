#pragma once

#include <string>
#include "Settings.h"

namespace deadaim {

class SaveSystem {
public:
    explicit SaveSystem(std::string filePath = "savegame.json");

    void load();

    bool submitScore(int score);

    int getHighScore() const;
    const Settings& getSettings() const;
    void updateSettings(const Settings& settings);
private:
    void save() const;

    std::string m_filePath;
    int m_highScore = 0;
    Settings m_settings;
};

} 