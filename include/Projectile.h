#pragma once

#include "IGameObject.h"
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector3.hpp>
#include <vector>

namespace deadaim {

// Everything distinguishing one weapon's projectile from another's.
// All sizes and speeds are WORLD units -- the projection decides what
// they look like on screen at any given depth.
struct ProjectileConfig {
    float speed = 40.f;                    // world units per second
    int damage = 20;
    sf::Color color = sf::Color::Yellow;
    sf::Vector2f worldSize{0.06f, 0.022f}; // length x thickness, world units
    float collisionRadius = 0.06f;         // world units
    int pierceCount = 1;
    float explosionRadius = 0.f;           // world units; 0 = no area damage
};

// Purpose: a shot travelling through world space, projected to screen
// each frame by the shared camera.
//
// Responsibilities: integrate a world-space position; expire when it
// leaves the play volume; derive its screen position, size and rotation
// from the projection; carry damage; remember which zombies it has hit.
//
// Why world space: zombies live in world coordinates, so collision must
// too. A screen-space projectile could only ever approximate a hit
// against a projected zombie -- a coordinate-system mismatch, not a
// tuning problem.
class Projectile : public IGameObject {
public:
    Projectile(sf::Vector3f origin, sf::Vector3f direction, const ProjectileConfig& config);

    void update(float dt) override;
    void render(Renderer& renderer) const override;
    bool isAlive() const override;

    sf::Vector3f getWorldPosition() const;
    sf::Vector3f getPreviousWorldPosition() const; // start of this tick's sweep
    float getWorldRadius() const;
    int getDamage() const;
    float getExplosionRadius() const;

    bool hasHit(unsigned int zombieId) const;
    void registerHit(unsigned int zombieId); // spends one pierce
    void consume();

private:
    void refreshVisual();

    sf::RectangleShape m_shape;
    sf::Vector3f m_position;
    sf::Vector3f m_previousPosition;
    sf::Vector3f m_velocity;
    sf::Vector2f m_worldSize;
    int m_damage;
    float m_worldRadius;
    float m_explosionRadius;
    int m_piercesRemaining;
    bool m_expired = false;
    std::vector<unsigned int> m_alreadyHit;

    // Screen-size guards. Without a maximum, a projectile at the muzzle
    // (z = 1) would render at focal-length scale and fill the screen;
    // without a minimum, a distant one would shrink below a pixel.
    static constexpr float kMinScreenLength = 3.f;
    static constexpr float kMinScreenThickness = 2.f;
    static constexpr float kMaxScreenLength = 160.f;
    static constexpr float kMaxScreenThickness = 90.f;
};

} // namespace deadaim