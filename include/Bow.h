#pragma once

#include "IWeapon.h"

namespace deadaim {


class Bow : public IWeapon {
public:
    void update(float dt) override;
    std::unique_ptr<Projectile> tryFire(const AimRay& ray) override;
    const char* getName() const override { return "Bow"; }

private:
    float m_cooldownTimer = 0.f;

    static constexpr float kCooldownSeconds = 0.6f;
};

} 