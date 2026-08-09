#include "Bow.h"
#include "Projectile.h"
#include <cmath>

namespace deadaim {

namespace {
const ProjectileConfig kArrowConfig{
    22.f,                       // speed: visibly slower than a bullet
    100,                        // damage: one-shots early zombies
    sf::Color(160, 255, 220),   // pale cyan
    {0.40f, 0.035f},            // world size: long, thin shaft
    0.09f,                      // collision radius
    5,                          // pierce: up to five zombies in a line
    0.f
};
}

void Bow::update(float dt) {
    if (m_cooldownTimer > 0.f) {
        m_cooldownTimer -= dt;
    }
}

std::unique_ptr<Projectile> Bow::tryFire(const AimRay& ray) {
    if (m_cooldownTimer > 0.f) {
        return nullptr;
    }
    m_cooldownTimer = kCooldownSeconds;
    return std::make_unique<Projectile>(ray, kArrowConfig);
}

} 