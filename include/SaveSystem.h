#pragma once

#include <string>

namespace deadaim {

// Purpose: persists the small amount of state that should survive between
// runs -- currently the high score, per the spec's "Local save: high
// score, settings. No online account."
//
// Responsibilities: read the save file at startup (treating "no file yet"
// as normal); track the best score seen; write the file when, and only
// when, a new best is achieved.
//
// Dependencies: nlohmann/json (already in the build for VisionClient).
//
// Future extension points: Settings (resolution, fullscreen, volume,
// sensitivity) go into the same JSON object as additional keys -- load()
// and save() gain fields, nothing else changes.
class SaveSystem {
public:
    explicit SaveSystem(std::string filePath = "savegame.json");

    void load();

    // Returns true if this beat the stored high score (and therefore
    // wrote the file). The Game Over screen uses that to celebrate it.
    bool submitScore(int score);

    int getHighScore() const;

private:
    void save() const;

    std::string m_filePath;
    int m_highScore = 0;
};

} // namespace deadaim