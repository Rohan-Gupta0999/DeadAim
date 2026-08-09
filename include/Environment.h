#pragma once

#include <SFML/Graphics.hpp>

namespace deadaim {

class Renderer;

class Environment {
public:
    Environment();

    void update(float dt);
    void render(Renderer& renderer) const;

private:
    void rebuildFog();

    sf::VertexArray m_sky;
    sf::VertexArray m_ground;
    sf::VertexArray m_horizonGlow;

    sf::VertexArray m_fogUpper;
    sf::VertexArray m_fogLower;

    sf::VertexArray m_vignetteTop;
    sf::VertexArray m_vignetteBottom;
    sf::VertexArray m_vignetteLeft;
    sf::VertexArray m_vignetteRight;
    sf::VertexArray m_groundLines;

    float m_time = 0.f;
};

} // namespace deadaim