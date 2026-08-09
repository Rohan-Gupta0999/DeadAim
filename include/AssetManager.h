#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

namespace deadaim {

class AssetManager {
public:
    AssetManager() = default;

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    sf::Texture& getTexture(const std::string& path);
    sf::Font* getFont(const std::string& path);

private:
    sf::Texture& getFallbackTexture();

    std::unordered_map<std::string, sf::Texture> m_textures;
    std::unordered_map<std::string, sf::Font> m_fonts;
    sf::Texture m_fallbackTexture;
    bool m_fallbackReady = false;
};

}