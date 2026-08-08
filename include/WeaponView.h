#pragma once

#include "IWeapon.h"
#include <SFML/Graphics.hpp>

namespace deadaim {

class Renderer;
class AssetManager;

// Purpose: the first-person view of the player's own hands and weapon,
// anchored to the bottom of the screen.
//
// Owned by Application rather than Player, following the same pattern as
// HUD: it needs AssetManager, and Player/Scene stay free of asset
// dependencies.
//
// Expects: assets/textures/hands_gun.png, hands_bow.png,
// hands_fireball.png -- each drawn as if looking down at your own hands.
// Missing files fall back to the magenta placeholder.
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
    float m_recoil = 0.f;   // 1 at the moment of firing, decaying to 0

    // The weapon is bolted to the camera. The camera never moves, so the
    // weapon never translates -- the crosshair is a cursor, not a look
    // direction, and following it is what made the weapon feel detached.
    static constexpr float kTargetHeight = 300.f;
    // Bottom-centre with a modest right-hand offset. 1090 is 130px right
    // of the 960 centre -- enough to read as a right-handed grip, small
    // enough that the sprite stays fully on screen and leaves the lower
    // centre clear for approaching zombies.
    static constexpr float kAnchorX = 1090.f;
    static constexpr float kAnchorBottomOffset = 30.f; // overhang past the edge

    // Idle bob: breathing, essentially. Small enough that you register it
    // as life rather than movement.
    static constexpr float kBobSpeed = 1.6f;
    static constexpr float kBobAmplitudeX = 5.f;
    static constexpr float kBobAmplitudeY = 4.f;

    // Recoil: a downward kick that snaps back. This is what actually
    // connects the weapon to the act of shooting.
    static constexpr float kRecoilKick = 26.f;
    static constexpr float kRecoilRecoverySeconds = 0.14f;
};

} // namespace deadaim