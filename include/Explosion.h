#pragma once

#include "IGameObject.h"
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector3.hpp>

namespace deadaim {


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

} 