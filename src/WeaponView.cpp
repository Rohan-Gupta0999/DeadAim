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
    m_muzzleFlash.setPointCount(6);
    m_muzzleFlash.setRadius(kFlashRadius);
    m_muzzleFlash.setOrigin({kFlashRadius, kFlashRadius});
    m_muzzleFlash.setFillColor(sf::Color(255, 220, 130, 0));
}

void WeaponView::update(float dt, WeaponType weapon, bool justFired) {
    if (weapon != m_currentWeapon) {
        m_currentWeapon = weapon;
        const sf::Texture* texture = m_gunTexture;
        if (weapon == WeaponType::Bow)      texture = m_bowTexture;
        if (weapon == WeaponType::Fireball) texture = m_fireballTexture;
        m_sprite.setTexture(*texture, true); 
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

    
    float bobX = std::sin(m_idleTime * kBobSpeed) * kBobAmplitudeX;
    float bobY = std::sin(m_idleTime * kBobSpeed * 2.f) * kBobAmplitudeY;
    float kick = m_recoil * kRecoilKick;

    
    m_sprite.setPosition({kAnchorX + bobX,
                          perspective::kDesignHeight + kAnchorBottomOffset + bobY + kick});
    if (justFired) {
    m_flashTimer = kFlashSeconds;
}

m_flashTimer = std::max(
    0.f,
    m_flashTimer - dt
);

if (m_flashTimer > 0.f) {
    float intensity =
        m_flashTimer / kFlashSeconds;

    float radius =
        kFlashRadius *
        (0.55f + 0.45f * intensity);

    m_muzzleFlash.setRadius(radius);
    m_muzzleFlash.setOrigin({radius, radius});

    m_muzzleFlash.setFillColor(
        sf::Color(
            255,
            222,
            140,
            static_cast<std::uint8_t>(
                235.f * intensity
            )
        )
    );

    m_muzzleFlash.setPosition({
        kAnchorX + kFlashOffsetX + bobX,

        perspective::kDesignHeight
        + kAnchorBottomOffset
        - kFlashOffsetY
        + bobY
        + kick
    });
}
                        }
void WeaponView::render(Renderer& renderer) const {
    renderer.submit(m_sprite, RenderLayer::Weapon);

    if (m_flashTimer > 0.f) {
        renderer.submit(
            m_muzzleFlash,
            RenderLayer::Weapon,
            1.f
        );
    }
}
}