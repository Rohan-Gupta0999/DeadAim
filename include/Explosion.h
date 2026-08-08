#pragma once

#include "IGameObject.h"
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector3.hpp>

namespace deadaim {

// Purpose: purely visual expanding ring marking a fireball detonation.
//
// Deliberately carries no damage: CollisionSystem applies the area
// damage instantly at the moment of impact. Splitting them means the
// visual can be retimed or replaced with a particle effect later
// without any risk of changing what the blast actually hits.
//
// CollisionSystem ignores this automatically -- it identifies objects by
// dynamic_cast to Projectile or Zombie, and this is neither.
class Explosion : public IGameObject {
public:
    Explosion(sf::Vector3f worldPosition, float worldRadius);

    void update(float dt) override;
    void render(Renderer& renderer) const override;
    bool isAlive() const override;

private:
    sf::CircleShape m_ring;
    sf::Vector3f m_worldPosition;
    float m_worldRadius;
    float m_elapsed = 0.f;

    static constexpr float kDurationSeconds = 0.35f;
};

} // namespace deadaim