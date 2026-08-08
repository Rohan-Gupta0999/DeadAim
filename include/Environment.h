#pragma once

#include <SFML/Graphics.hpp>

namespace deadaim {

class Renderer;

// Purpose: the static level's atmosphere -- the sky/ground backdrop the
// action happens against, plus horizon fog and an edge vignette.
//
// Responsibilities: build the backdrop geometry once at construction;
// rebuild only the fog each frame so its density can drift; submit
// backdrop to the Background layer and fog/vignette to the Fog layer.
//
// Dependencies: Renderer. Deliberately none on AssetManager -- every
// element here is procedural geometry, so the environment looks correct
// before any art exists.
//
// Future extension points: a background texture would be submitted to
// the Background layer before these gradients, with the sky/ground
// gradients kept underneath as a fallback. Parallax layers or a
// silhouetted treeline would slot in the same way.
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