#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

namespace deadaim {

// Purpose: loads assets from disk exactly once each and hands out
// references to the cached result.
//
// Responsibilities: load-and-cache textures and fonts by path; return a
// visually obvious fallback texture (and log) when a texture is missing.
//
// Dependencies: SFML (Graphics module).
//
// Note on the API asymmetry: getTexture() returns a reference because a
// missing texture can be substituted with a synthesized magenta square.
// getFont() returns a pointer because there is no way to synthesize a
// font -- callers must handle nullptr rather than be handed a lie.
//
// Contract: returned references/pointers stay valid for as long as this
// AssetManager exists. Anything holding one must not outlive it.
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

} // namespace deadaim