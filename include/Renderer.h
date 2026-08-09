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
    
    void submit(const sf::Drawable& drawable, RenderLayer layer, float sortKey = 0.f);
    void endFrame();

    
    sf::Vector2f mapPixelToDesignSpace(sf::Vector2i pixel) const;

private:
    struct Submission {
        const sf::Drawable* drawable;
        float sortKey;
    };

    sf::RenderTarget& m_target;
    sf::View m_designView;
    std::array<std::vector<Submission>, static_cast<std::size_t>(RenderLayer::Count)> m_layers;

}; }