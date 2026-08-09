#include "AssetManager.h"
#include <iostream>
#include <utility>

namespace deadaim {

sf::Texture& AssetManager::getTexture(const std::string& path) {
    auto it = m_textures.find(path);
    if (it != m_textures.end()) {
        return it->second;
    }

    sf::Texture texture;
    if (!texture.loadFromFile(path)) {
        std::cerr << "[AssetManager] Failed to load texture: " << path
                  << " -- using fallback.\n";
        return getFallbackTexture();
    }

    auto [insertedIt, inserted] = m_textures.emplace(path, std::move(texture));
    return insertedIt->second;
}

sf::Font* AssetManager::getFont(const std::string& path) {
    auto it = m_fonts.find(path);
    if (it != m_fonts.end()) {
        return &it->second;
    }

    sf::Font font;
    if (!font.openFromFile(path)) {
        std::cerr << "[AssetManager] Failed to load font: " << path << "\n";
        return nullptr;
    }

    auto [insertedIt, inserted] = m_fonts.emplace(path, std::move(font));
    return &insertedIt->second;
}



sf::Texture& AssetManager::getFallbackTexture() {
    if (!m_fallbackReady) {
        sf::Image image({64, 64}, sf::Color(255, 0, 255));
        (void)m_fallbackTexture.loadFromImage(image);
        m_fallbackReady = true;
    }
    return m_fallbackTexture;
}

} 