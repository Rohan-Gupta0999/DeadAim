#include "WeaponView.h"
#include "AssetManager.h"
#include "Renderer.h"
#include "RenderLayer.h"
#include "Perspective.h"
#include <algorithm>
#include <cmath>

namespace deadaim {

WeaponView::WeaponView(AssetManager& assets)
    : m_sprite(assets.getTexture("assets/textures/hands_gun.png"))
    , m_gunTexture(&assets.getTexture("assets/textures/hands_gun.png"))
    , m_bowTexture(&assets.getTexture("assets/textures/hands_bow.png"))
    , m_fireballTexture(&assets.getTexture("assets/textures/hands_fireball.png"))
{
    update(0.f, WeaponType::Gun, false);
}

void WeaponView::update(float dt, WeaponType weapon, bool justFired) {
    if (weapon != m_currentWeapon) {
        m_currentWeapon = weapon;
        const sf::Texture* texture = m_gunTexture;
        if (weapon == WeaponType::Bow)      texture = m_bowTexture;
        if (weapon == WeaponType::Fireball) texture = m_fireballTexture;
        m_sprite.setTexture(*texture, true); // resetRect: sizes differ per weapon
    }

    sf::FloatRect bounds = m_sprite.getLocalBounds();
    float baseHeight = std::max(bounds.size.y, 1.f);
    float scale = kTargetHeight / baseHeight;
    m_sprite.setScale({scale, scale});
    m_sprite.setOrigin({bounds.size.x / 2.f, bounds.size.y});

    m_idleTime += dt;
    if (justFired) {
        m_recoil = 1.f;
    }
    m_recoil = std::max(0.f, m_recoil - dt / kRecoilRecoverySeconds);

    // Lissajous bob: horizontal and vertical on different periods, so the
    // motion traces a slow figure-eight rather than an obvious straight
    // oscillation.
    float bobX = std::sin(m_idleTime * kBobSpeed) * kBobAmplitudeX;
    float bobY = std::sin(m_idleTime * kBobSpeed * 2.f) * kBobAmplitudeY;
    float kick = m_recoil * kRecoilKick;

    // Fixed anchor. No crosshair term anywhere in this expression.
    m_sprite.setPosition({kAnchorX + bobX,
                          perspective::kDesignHeight + kAnchorBottomOffset + bobY + kick});
}

void WeaponView::render(Renderer& renderer) const {
    renderer.submit(m_sprite, RenderLayer::Weapon);
}

} // namespace deadaim