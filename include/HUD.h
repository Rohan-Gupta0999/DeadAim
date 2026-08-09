#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>

namespace deadaim {

class Renderer;
class AssetManager;

// Everything the HUD needs to draw one frame. Passed in by Application,
// which is the only place that can see every system at once. Keeping this
// a plain struct means HUD has no dependency on Scene, WaveSystem, or
// Player -- and adding a field later (bow draw, fireball charge) touches
// only this struct and the two functions that use it.
struct HudState {
    int health = 100;
    int maxHealth = 100;
    int score = 0;
    int wave = 1;
    std::string weaponName = "Gun";
    bool visionTracking = false;
    bool inGunMode = false;
    bool bowDrawn = false;
    float fireballCharge = 0.f; // 0..1; the bar is hidden at 0
};

class HUD {
public:
    explicit HUD(AssetManager& assets);

    void update(const HudState& state);
    void render(Renderer& renderer) const;

private:
    bool m_textReady = false;

    std::optional<sf::Text> m_scoreText;
    std::optional<sf::Text> m_waveText;
    std::optional<sf::Text> m_weaponText;
    std::optional<sf::Text> m_healthText;
    std::optional<sf::Text> m_trackingText;

    sf::RectangleShape m_healthBarBackground;
    sf::RectangleShape m_healthBarFill;

    sf::RectangleShape m_chargeBarBackground;
    sf::RectangleShape m_chargeBarFill;
    bool m_showChargeBar = false;

    static constexpr float kHealthBarWidth = 400.f;
    static constexpr float kHealthBarHeight = 36.f;
};

} 