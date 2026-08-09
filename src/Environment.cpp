#include "Environment.h"
#include "Renderer.h"
#include "RenderLayer.h"
#include <cmath>
#include "Perspective.h"

namespace deadaim {

namespace {
constexpr float kDesignWidth = 1920.f;
constexpr float kDesignHeight = 1080.f;

// Where the ground meets the sky. Zombies spawn at y=50 (well above
// this) and walk down toward the player, so this line is the visual
// anchor for "far away".
constexpr float kHorizonY = 400.f;

// A rectangle whose colour interpolates top-to-bottom. Four vertices in
// a triangle strip, ordered TL, TR, BL, BR -- the GPU does the blend, so
// a full-screen gradient costs the same as a flat rectangle.
sf::VertexArray verticalGradient(float x, float y, float w, float h,
                                  sf::Color top, sf::Color bottom) {
    sf::VertexArray strip(sf::PrimitiveType::TriangleStrip, 4);
    strip[0].position = {x,     y};     strip[0].color = top;
    strip[1].position = {x + w, y};     strip[1].color = top;
    strip[2].position = {x,     y + h}; strip[2].color = bottom;
    strip[3].position = {x + w, y + h}; strip[3].color = bottom;
    return strip;
}

// Same idea, interpolating left-to-right. Vertex order is TL, BL, TR, BR
// here -- a triangle strip has to zigzag between the two edges it spans.
sf::VertexArray horizontalGradient(float x, float y, float w, float h,
                                    sf::Color left, sf::Color right) {
    sf::VertexArray strip(sf::PrimitiveType::TriangleStrip, 4);
    strip[0].position = {x,     y};     strip[0].color = left;
    strip[1].position = {x,     y + h}; strip[1].color = left;
    strip[2].position = {x + w, y};     strip[2].color = right;
    strip[3].position = {x + w, y + h}; strip[3].color = right;
    return strip;
}

const sf::Color kFogColour(120, 130, 140);
constexpr float kFogBaseAlpha = 90.f;
constexpr float kFogDriftAmount = 22.f;   // how far density swings either side of base
constexpr float kFogDriftSpeed = 0.35f;   // radians/sec -- deliberately slow
} // namespace

Environment::Environment() {
    // Night sky: near-black overhead, lifting slightly toward the horizon
    // the way real sky does from distant light scatter.
    m_sky = verticalGradient(0.f, 0.f, kDesignWidth, kHorizonY,
                             sf::Color(5, 7, 11), sf::Color(26, 29, 34));

    // Ground: darkest at the horizon, warming subtly toward the bottom of
    // the screen -- reads as light falling from the player's position,
    // which is the "simple lighting" the spec asks for without any
    // actual lighting maths.
    m_ground = verticalGradient(0.f, kHorizonY, kDesignWidth, kDesignHeight - kHorizonY,
                                sf::Color(19, 19, 22), sf::Color(38, 35, 31));

    // A thin brighter band exactly on the horizon line. Without it the
    // sky/ground meeting point reads as a flat seam.
    m_horizonGlow = verticalGradient(0.f, kHorizonY - 40.f, kDesignWidth, 80.f,
                                     sf::Color(46, 52, 60, 0), sf::Color(46, 52, 60, 130));

    // Vignette: pure black fading inward from each edge. Four strips
    // approximate a radial falloff closely enough that the difference is
    // invisible in motion, and cost four vertices each instead of a
    // shader or a full-screen alpha PNG.
    m_vignetteTop = verticalGradient(0.f, 0.f, kDesignWidth, 220.f,
                                     sf::Color(0, 0, 0, 150), sf::Color(0, 0, 0, 0));
    m_vignetteBottom = verticalGradient(0.f, kDesignHeight - 220.f, kDesignWidth, 220.f,
                                        sf::Color(0, 0, 0, 0), sf::Color(0, 0, 0, 150));
    m_vignetteLeft = horizontalGradient(0.f, 0.f, 320.f, kDesignHeight,
                                        sf::Color(0, 0, 0, 160), sf::Color(0, 0, 0, 0));
    m_vignetteRight = horizontalGradient(kDesignWidth - 320.f, 0.f, 320.f, kDesignHeight,
                                         sf::Color(0, 0, 0, 0), sf::Color(0, 0, 0, 160));

    rebuildFog();

    m_groundLines.setPrimitiveType(sf::PrimitiveType::Lines);

    const sf::Color lineFar(70, 78, 88, 0);    
    const sf::Color lineNear(70, 78, 88, 55);  

    constexpr int kRadialCount = 17;
    for (int i = 0; i < kRadialCount; ++i) {
        float t = static_cast<float>(i) / (kRadialCount - 1);
        
        float bottomX = -900.f + t * (kDesignWidth + 1800.f);

        sf::Vertex top;
        top.position = {perspective::kVanishingX, perspective::kHorizonY};
        top.color = lineFar;
        sf::Vertex bottom;
        bottom.position = {bottomX, kDesignHeight};
        bottom.color = lineNear;

        m_groundLines.append(top);
        m_groundLines.append(bottom);
    }


    constexpr int kBandCount = 11;
    constexpr float kNearestBandDepth = 1.4f; 
    for (int i = 0; i < kBandCount; ++i) {
        float t = static_cast<float>(i) / (kBandCount - 1);
        float z = perspective::kSpawnDepth
                + t * (kNearestBandDepth - perspective::kSpawnDepth);
        float y = perspective::projectGroundY(z);
        auto alpha = static_cast<std::uint8_t>(12.f + 44.f * t);

        sf::Vertex left;
        left.position = {0.f, y};
        left.color = sf::Color(70, 78, 88, alpha);
        sf::Vertex right;
        right.position = {kDesignWidth, y};
        right.color = sf::Color(70, 78, 88, alpha);

        m_groundLines.append(left);
        m_groundLines.append(right);
    }
}
void Environment::update(float dt) {
    m_time += dt;
    rebuildFog();
}

void Environment::rebuildFog() {
 
    float drift = std::sin(m_time * kFogDriftSpeed);
    auto peak = static_cast<std::uint8_t>(kFogBaseAlpha + kFogDriftAmount * drift);

    sf::Color dense(kFogColour.r, kFogColour.g, kFogColour.b, peak);
    sf::Color clear(kFogColour.r, kFogColour.g, kFogColour.b, 0);


    m_fogUpper = verticalGradient(0.f, 260.f, kDesignWidth, kHorizonY - 260.f, clear, dense);
    m_fogLower = verticalGradient(0.f, kHorizonY, kDesignWidth, 320.f, dense, clear);
}

void Environment::render(Renderer& renderer) const {
    renderer.submit(m_sky, RenderLayer::Background);
    renderer.submit(m_ground, RenderLayer::Background);
    renderer.submit(m_groundLines, RenderLayer::Background);
    renderer.submit(m_horizonGlow, RenderLayer::Background);

    renderer.submit(m_fogUpper, RenderLayer::Fog);
    renderer.submit(m_fogLower, RenderLayer::Fog);

    renderer.submit(m_vignetteTop, RenderLayer::Fog);
    renderer.submit(m_vignetteBottom, RenderLayer::Fog);
    renderer.submit(m_vignetteLeft, RenderLayer::Fog);
    renderer.submit(m_vignetteRight, RenderLayer::Fog);
}

} 