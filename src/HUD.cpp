#include "HUD.h"
#include "AssetManager.h"
#include "Renderer.h"
#include "RenderLayer.h"
#include <algorithm>
#include <iostream>

namespace deadaim {

namespace {
const sf::Vector2f kHealthBarPosition{40.f, 990.f};
constexpr float kScoreY = 30.f;
constexpr float kWaveY = 80.f;
constexpr float kWeaponY = 880.f;                      
const sf::Vector2f kChargeBarPosition{40.f, 935.f};
constexpr float kChargeBarHeight = 22.f;
constexpr float kLeftMargin = 40.f;
}

HUD::HUD(AssetManager& assets) {
    
    m_healthBarBackground.setSize({kHealthBarWidth, kHealthBarHeight});
    m_healthBarBackground.setPosition(kHealthBarPosition);
    m_healthBarBackground.setFillColor(sf::Color(60, 60, 60));
    m_healthBarBackground.setOutlineColor(sf::Color(180, 180, 180));
    m_healthBarBackground.setOutlineThickness(2.f);

    m_healthBarFill.setSize({kHealthBarWidth, kHealthBarHeight});
    m_healthBarFill.setPosition(kHealthBarPosition);
    m_healthBarFill.setFillColor(sf::Color(200, 40, 40));

    sf::Font* font = assets.getFont("assets/fonts/main.ttf");
    if (font == nullptr) {
        std::cerr << "[HUD] No font found -- HUD text disabled. "
                     "Place a .ttf at assets/fonts/main.ttf\n";
        return; // m_textReady stays false; the health bar still draws
    }

    m_scoreText.emplace(*font, "", 34);
    m_scoreText->setPosition({kLeftMargin, kScoreY});
    m_scoreText->setFillColor(sf::Color::White);

    m_waveText.emplace(*font, "", 28);
    m_waveText->setPosition({kLeftMargin, kWaveY});
    m_waveText->setFillColor(sf::Color(200, 200, 200));

    m_weaponText.emplace(*font, "", 28);
    m_weaponText->setPosition({kLeftMargin, kWeaponY});
    m_weaponText->setFillColor(sf::Color(220, 220, 120));

    m_healthText.emplace(*font, "", 28);
    m_healthText->setPosition({kLeftMargin + kHealthBarWidth + 20.f, kHealthBarPosition.y + 2.f});
    m_healthText->setFillColor(sf::Color::White);

    m_trackingText.emplace(*font, "", 24);
    m_trackingText->setPosition({1480.f, kScoreY});

    m_textReady = true;

    m_chargeBarBackground.setSize({kHealthBarWidth, kChargeBarHeight});
        m_chargeBarBackground.setPosition(kChargeBarPosition);
        m_chargeBarBackground.setFillColor(sf::Color(60, 45, 30));
        m_chargeBarBackground.setOutlineColor(sf::Color(180, 140, 80));
        m_chargeBarBackground.setOutlineThickness(2.f);

        m_chargeBarFill.setSize({kHealthBarWidth, kChargeBarHeight});
        m_chargeBarFill.setPosition(kChargeBarPosition);

}

void HUD::update(const HudState& state) {
    float healthFraction = (state.maxHealth > 0)
        ? static_cast<float>(state.health) / static_cast<float>(state.maxHealth)
        : 0.f;
    healthFraction = std::clamp(healthFraction, 0.f, 1.f);
    m_healthBarFill.setSize({kHealthBarWidth * healthFraction, kHealthBarHeight});


    m_showChargeBar = state.fireballCharge > 0.f;
    if (m_showChargeBar) {
        float charge = std::clamp(state.fireballCharge, 0.f, 1.f);
        m_chargeBarFill.setSize({kHealthBarWidth * charge, kChargeBarHeight});
        
        m_chargeBarFill.setFillColor(charge >= 1.f ? sf::Color(255, 230, 120)
                                                   : sf::Color(220, 110, 40));
    }

    if (!m_textReady) {
        return;
    }

    m_scoreText->setString("SCORE  " + std::to_string(state.score));
    m_waveText->setString("WAVE  " + std::to_string(state.wave));
    m_weaponText->setString("WEAPON  " + state.weaponName);
    m_healthText->setString(std::to_string(state.health));

    // Tracking status earns HUD space because in a gesture-controlled
    // game, "nothing is happening" and "my hand isn't detected" look
    // identical without it.
    if (!state.visionTracking) {
        m_trackingText->setString("HAND: NOT DETECTED");
        m_trackingText->setFillColor(sf::Color(200, 80, 80));
    } else if (state.bowDrawn) {
        m_trackingText->setString("HAND: BOW DRAWN");
        m_trackingText->setFillColor(sf::Color(240, 200, 80));
    } else if (state.fireballCharge > 0.f) {
        m_trackingText->setString("HAND: CHARGING");
        m_trackingText->setFillColor(sf::Color(255, 150, 60));
    } else if (state.inGunMode) {
        m_trackingText->setString("HAND: GUN MODE");
        m_trackingText->setFillColor(sf::Color(80, 220, 80));
    } else {
        m_trackingText->setString("HAND: TRACKED");
        m_trackingText->setFillColor(sf::Color(200, 200, 200));
    }
}

void HUD::render(Renderer& renderer) const {
    renderer.submit(m_healthBarBackground, RenderLayer::UI);
    renderer.submit(m_healthBarFill, RenderLayer::UI);

    if (m_showChargeBar) {
        renderer.submit(m_chargeBarBackground, RenderLayer::UI);
        renderer.submit(m_chargeBarFill, RenderLayer::UI);
    }

    if (!m_textReady) {
        return;
    }

    renderer.submit(*m_scoreText, RenderLayer::UI);
    renderer.submit(*m_waveText, RenderLayer::UI);
    renderer.submit(*m_weaponText, RenderLayer::UI);
    renderer.submit(*m_healthText, RenderLayer::UI);
    renderer.submit(*m_trackingText, RenderLayer::UI);
}

} 