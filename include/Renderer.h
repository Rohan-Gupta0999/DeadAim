#pragma once

#include "RenderLayer.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <cstddef>
#include <vector>

namespace deadaim {

class Renderer {
public:
    explicit Renderer(sf::RenderTarget& target);

    void beginFrame(const sf::Color& clearColor = sf::Color::Black);
    // sortKey orders drawables WITHIN a layer -- higher draws later, i.e.
    // in front. Used for depth sorting zombies so nearer ones occlude
    // farther ones. Default 0 keeps every existing call site unchanged.
    void submit(const sf::Drawable& drawable, RenderLayer layer, float sortKey = 0.f);
    void endFrame();

    // Converts a raw window pixel coordinate (e.g. from the mouse) into
    // the 1920x1080 design-space coordinates everything else is drawn
    // in — reuses the same View Renderer already owns, rather than
    // duplicating that math somewhere else.
    sf::Vector2f mapPixelToDesignSpace(sf::Vector2i pixel) const;

private:
    struct Submission {
        const sf::Drawable* drawable;
        float sortKey;
    };

    sf::RenderTarget& m_target;
    sf::View m_designView;
    std::array<std::vector<Submission>, static_cast<std::size_t>(RenderLayer::Count)> m_layers;

}; }// namespace deadaim