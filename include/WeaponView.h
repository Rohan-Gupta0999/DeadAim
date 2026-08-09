#pragma once

#include "IWeapon.h"
#include <SFML/Graphics.hpp>

namespace deadaim {

class Renderer;
class AssetManager;

class WeaponView {
public:
    explicit WeaponView(AssetManager& assets);

    void update(float dt, WeaponType weapon, bool justFired);
    void render(Renderer& renderer) const;

private:
    sf::Sprite m_sprite;
    const sf::Texture* m_gunTexture;
    const sf::Texture* m_bowTexture;
    const sf::Texture* m_fireballTexture;
   WeaponType m_currentWeapon = WeaponType::Gun;
    float m_idleTime = 0.f;
    float m_recoil = 0.f;   

    static constexpr float kTargetHeight = 300.f;
    
    static constexpr float kAnchorX = 1090.f;
    static constexpr float kAnchorBottomOffset = 30.f; 

    static constexpr float kBobSpeed = 1.6f;
    static constexpr float kBobAmplitudeX = 5.f;
    static constexpr float kBobAmplitudeY = 4.f;


    static constexpr float kRecoilKick = 26.f;
    static constexpr float kRecoilRecoverySeconds = 0.14f;
};

} 