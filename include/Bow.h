#pragma once

#include "IWeapon.h"

namespace deadaim {

// Purpose: slow, high-damage weapon whose arrow passes through multiple
// zombies instead of stopping at the first.
//
// Note on firing: Bow has no concept of "drawn" internally. The draw is
// a gesture state, and the release is detected as its falling edge in
// Application -- by the time tryFire() is called, the decision to shoot
// has already been made. Keeping that logic out of here means Bow stays
// a plain IWeapon with nothing gesture-specific baked in, and the mouse
// fallback still works identically.
//
// The cooldown is pure insurance: a release can't physically be spammed,
// but a flickering pinch could otherwise loose two arrows at once.
class Bow : public IWeapon {
public:
    void update(float dt) override;
    std::unique_ptr<Projectile> tryFire(sf::Vector3f muzzleWorld,
                                        sf::Vector3f directionWorld) override;
    const char* getName() const override { return "Bow"; }

private:
    float m_cooldownTimer = 0.f;

    static constexpr float kCooldownSeconds = 0.6f;
};

} // namespace deadaim