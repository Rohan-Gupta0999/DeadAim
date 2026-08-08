#include "Projectile.h"
#include "Renderer.h"
#include "Perspective.h"
#include <algorithm>
#include <cmath>

namespace deadaim {

namespace {
constexpr float kRadToDeg = 180.f / 3.14159265358979f;
}

Projectile::Projectile(sf::Vector3f origin, sf::Vector3f direction,
                        const ProjectileConfig& config)
    : m_position(origin)
    , m_previousPosition(origin)
    , m_velocity(perspective::normalized(direction) * config.speed)
    , m_worldSize(config.worldSize)
    , m_damage(config.damage)
    , m_worldRadius(config.collisionRadius)
    , m_explosionRadius(config.explosionRadius)
    , m_piercesRemaining(config.pierceCount)
{
    m_shape.setFillColor(config.color);
    m_alreadyHit.reserve(static_cast<std::size_t>(config.pierceCount));
    refreshVisual(); // project into place before the first frame draws
}

void Projectile::update(float dt) {
    m_previousPosition = m_position;
    m_position += m_velocity * dt;

    // Depth bounds. z can never go negative here -- every aim direction
    // has a positive z component, because the aim target is unprojected
    // at spawn depth, always well beyond the muzzle.
    if (m_position.z > perspective::kMaxProjectileDepth || m_position.z < 0.2f) {
        m_expired = true;
        return;
    }

    // Screen bounds, with margin so something just off-frame isn't culled
    // while still visually relevant.
    sf::Vector2f screen = perspective::project(m_position);
    if (screen.x < -300.f || screen.x > perspective::kDesignWidth + 300.f ||
        screen.y < -300.f || screen.y > perspective::kDesignHeight + 300.f) {
        m_expired = true;
        return;
    }

    refreshVisual();
}

void Projectile::refreshVisual() {
    sf::Vector2f screen = perspective::project(m_position);
    float scale = perspective::projectScale(m_position.z);

    sf::Vector2f size{
        std::clamp(m_worldSize.x * scale, kMinScreenLength, kMaxScreenLength),
        std::clamp(m_worldSize.y * scale, kMinScreenThickness, kMaxScreenThickness)
    };

    m_shape.setSize(size);
    m_shape.setOrigin({size.x * 0.5f, size.y * 0.5f});
    m_shape.setPosition(screen);

    // Rotation comes from projecting a point slightly ahead along the
    // velocity, not from the raw 3D direction: what we need is the
    // travel direction ON SCREEN, which perspective bends. This is what
    // makes tracers streak toward the vanishing point correctly.
    sf::Vector2f aheadScreen = perspective::project(m_position + m_velocity * 0.02f);
    sf::Vector2f delta = aheadScreen - screen;
    if (delta.x * delta.x + delta.y * delta.y > 0.0001f) {
        m_shape.setRotation(sf::degrees(std::atan2(delta.y, delta.x) * kRadToDeg));
    }
}

void Projectile::render(Renderer& renderer) const {
    // Shared sort key with Zombie: larger scale means nearer means drawn
    // in front, so projectiles interleave correctly with the horde.
    renderer.submit(m_shape, RenderLayer::World, perspective::projectScale(m_position.z));
}

bool Projectile::isAlive() const {
    return !m_expired;
}

sf::Vector3f Projectile::getWorldPosition() const {
    return m_position;
}

sf::Vector3f Projectile::getPreviousWorldPosition() const {
    return m_previousPosition;
}

float Projectile::getWorldRadius() const {
    return m_worldRadius;
}

int Projectile::getDamage() const {
    return m_damage;
}

float Projectile::getExplosionRadius() const {
    return m_explosionRadius;
}

bool Projectile::hasHit(unsigned int zombieId) const {
    return std::find(m_alreadyHit.begin(), m_alreadyHit.end(), zombieId) != m_alreadyHit.end();
}

void Projectile::registerHit(unsigned int zombieId) {
    m_alreadyHit.push_back(zombieId);
    --m_piercesRemaining;
    if (m_piercesRemaining <= 0) {
        m_expired = true;
    }
}

void Projectile::consume() {
    m_expired = true;
}

} // namespace deadaim