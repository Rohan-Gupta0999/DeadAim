#include "Gun.h"
#include "Projectile.h"

namespace deadaim {

namespace {
const ProjectileConfig kBulletConfig{
    40.f,                  // speed: ~0.43s to cross the field
    20,                    // damage
    sf::Color::Yellow,
    {0.06f, 0.022f},       // world size: a short tracer
    0.06f,                 // collision radius
    1,                     // pierce: stops at the first zombie
    0.f                    // no blast
};
}

Gun::Gun()
    : m_roundsInMagazine(kMagazineSize)
{
}

void Gun::update(float dt) {
    if (m_state == State::Ready) {
        return;
    }

    m_timer -= dt;
    if (m_timer <= 0.f) {
        if (m_state == State::Reloading) {
            m_roundsInMagazine = kMagazineSize;
        }
        m_state = State::Ready;
    }
}

std::unique_ptr<Projectile> Gun::tryFire(const AimRay& ray) {
    if (m_state != State::Ready) {
        return nullptr;
    }

    --m_roundsInMagazine;
    if (m_roundsInMagazine <= 0) {
        m_state = State::Reloading;
        m_timer = kReloadSeconds;
    } else {
        m_state = State::Cooldown;
        m_timer = kCooldownSeconds;
    }

    return std::make_unique<Projectile>(ray, kBulletConfig);
}

} 