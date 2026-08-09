#include "Explosion.h"
#include "Renderer.h"
#include <algorithm>
#include "Perspective.h"

namespace deadaim {

Explosion::Explosion(sf::Vector3f worldPosition, float worldRadius)
    : m_worldPosition(worldPosition)
    , m_worldRadius(worldRadius)
{
    m_ring.setFillColor(sf::Color::Transparent);
    m_ring.setOutlineThickness(6.f);
    m_ring.setPointCount(48);
    update(0.f);
}

void Explosion::update(float dt) {
    m_elapsed += dt;
    float progress = std::clamp(m_elapsed / kDurationSeconds, 0.f, 1.f);

   
    sf::Vector2f screen = perspective::project(m_worldPosition);
    float scale = perspective::projectScale(m_worldPosition.z);
    float radius = std::min(m_worldRadius * scale * progress, 700.f);

    m_ring.setRadius(radius);
   
    m_ring.setOrigin({radius, radius});
    m_ring.setPosition(screen);

    auto alpha = static_cast<std::uint8_t>(255.f * (1.f - progress));
    m_ring.setOutlineColor(sf::Color(255, 170, 60, alpha));
}

void Explosion::render(Renderer& renderer) const {
    renderer.submit(m_ring, RenderLayer::World,perspective::projectScale(m_worldPosition.z));
}
bool Explosion::isAlive() const {
    return m_elapsed < kDurationSeconds;
}

} 