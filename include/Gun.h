#pragma once

#include "IWeapon.h"

namespace deadaim {

class Gun : public IWeapon {
public:
    Gun();

    void update(float dt) override;
    std::unique_ptr<Projectile> tryFire(sf::Vector3f muzzleWorld,
                                        sf::Vector3f directionWorld) override;
    const char* getName() const override { return "Gun"; }

private:
    enum class State { Ready, Cooldown, Reloading };

    State m_state = State::Ready;
    float m_timer = 0.f;
    int m_roundsInMagazine;

    static constexpr float kCooldownSeconds = 0.15f;
    static constexpr float kReloadSeconds = 1.5f;
    static constexpr int kMagazineSize = 12;
};

} // namespace deadaim