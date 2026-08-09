#include "Renderer.h"
#include <algorithm>
namespace deadaim {

namespace {
constexpr float kDesignWidth = 1920.f;
constexpr float kDesignHeight = 1080.f;
}

Renderer::Renderer(sf::RenderTarget& target)
    : m_target(target)
    , m_designView(sf::FloatRect({0.f, 0.f}, {kDesignWidth, kDesignHeight}))
{
    m_target.setView(m_designView);
}

void Renderer::beginFrame(const sf::Color& clearColor) {
    m_target.clear(clearColor);
    for (auto& bucket : m_layers) {
        bucket.clear();
    }
}

void Renderer::submit(const sf::Drawable& drawable, RenderLayer layer, float sortKey) {
    m_layers[static_cast<std::size_t>(layer)].push_back({&drawable, sortKey});
}

void Renderer::endFrame() {
    for (auto& bucket : m_layers) {
        
        std::stable_sort(bucket.begin(), bucket.end(),
            [](const Submission& a, const Submission& b) {
                return a.sortKey < b.sortKey;
            });

        for (const Submission& submission : bucket) {
            m_target.draw(*submission.drawable);
        }
    }
}

sf::Vector2f Renderer::mapPixelToDesignSpace(sf::Vector2i pixel) const {
    return m_target.mapPixelToCoords(pixel, m_designView);
}

} 