#pragma once

#include <string>

namespace deadaim {

class SaveSystem {
public:
    explicit SaveSystem(std::string filePath = "savegame.json");

    void load();

    bool submitScore(int score);

    int getHighScore() const;

private:
    void save() const;

    std::string m_filePath;
    int m_highScore = 0;
};

} 