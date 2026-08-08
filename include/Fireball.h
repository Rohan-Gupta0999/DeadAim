#pragma once

#include "IWeapon.h"

namespace deadaim {

// Purpose: unlimited-ammo energy weapon dealing area damage on impact.
//
// Note on the cooldown: it's short, because the real gate is the charge
// time Application enforces before it ever calls tryFire(). Same split
// as the Bow -- gesture-derived timing belongs to Application, weapon
// timing belongs here.
class Fireball : public IWeapon {
public:
    void update(float dt) override;
    std::unique_ptr<Projectile> tryFire(sf::Vector3f muzzleWorld,
                                        sf::Vector3f directionWorld) override;
    const char* getName() const override { return "Fireball"; }

private:
    float m_cooldownTimer = 0.f;

    static constexpr float kCooldownSeconds = 0.4f;
};

} // namespace deadaim