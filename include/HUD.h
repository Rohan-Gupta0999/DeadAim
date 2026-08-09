#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>

namespace deadaim {

class Renderer;
class AssetManager;


struct HudState {
    int health = 100;
    int maxHealth = 100;
    int score = 0;
    int wave = 1;
    std::string weaponName = "Gun";
    bool visionTracking = false;
    bool inGunMode = false;
    bool bowDrawn = false;
    float fireballCharge = 0.f; 
    bool playerJustHurt = false;
};

class HUD {
public:
    explicit HUD(AssetManager& assets);

    void update(float dt, const HudState& state);
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
    sf::RectangleShape m_damageFlash;
    float m_damageFlashTimer = 0.f;

    static constexpr float kDamageFlashSeconds = 0.3f;
    static constexpr float kHealthBarWidth = 400.f;
    static constexpr float kHealthBarHeight = 36.f;
};

} 