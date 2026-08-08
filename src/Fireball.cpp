#include "Fireball.h"
#include "Projectile.h"
#include <cmath>

namespace deadaim {

namespace {
const ProjectileConfig kFireballConfig{
    14.f,                       // speed: slowest of the three
    90,                         // damage, applied to everything in the blast
    sf::Color(255, 140, 40),
    {0.34f, 0.34f},             // world size: chunky, roughly square
    0.14f,                      // collision radius
    1,                          // detonates on first contact
    1.8f                        // blast radius in WORLD units
};
}

void Fireball::update(float dt) {
    if (m_cooldownTimer > 0.f) {
        m_cooldownTimer -= dt;
    }
}

std::unique_ptr<Projectile> Fireball::tryFire(sf::Vector3f muzzleWorld,
                                               sf::Vector3f directionWorld) {
    if (m_cooldownTimer > 0.f) {
        return nullptr;
    }

    m_cooldownTimer = kCooldownSeconds;
    return std::make_unique<Projectile>(muzzleWorld, directionWorld, kFireballConfig);
}

} // namespace deadaim