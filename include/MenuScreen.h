#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>

namespace deadaim {

class Renderer;
class AssetManager;

// Everything the menu needs to display beyond which mode it's in.
// A struct rather than a growing parameter list, matching HudState.
struct MenuInfo {
    int finalScore = 0;
    int highScore = 0;
    bool isNewHighScore = false;
};

class MenuScreen {
public:
    enum class Mode { MainMenu, GameOver };
    enum class Action { None, StartGame, Quit };

    explicit MenuScreen(AssetManager& assets);

    void setMode(Mode mode, const MenuInfo& info = {});
    Action update(sf::Vector2f mouseDesignPosition, bool clicked);
    void render(Renderer& renderer) const;

private:
    struct Button {
        sf::RectangleShape shape;
        std::optional<sf::Text> label;

        bool contains(sf::Vector2f point) const {
            return shape.getGlobalBounds().contains(point);
        }
    };

    void buildButton(Button& button, sf::Font* font, const std::string& text, float y);

    bool m_textReady = false;
    sf::RectangleShape m_backdrop;
    std::optional<sf::Text> m_titleText;
    std::optional<sf::Text> m_subtitleText;
    std::optional<sf::Text> m_highScoreText;
    Button m_primaryButton;
    Button m_quitButton;

    static constexpr float kDesignWidth = 1920.f;
    static constexpr float kDesignHeight = 1080.f;
    static constexpr float kButtonWidth = 380.f;
    static constexpr float kButtonHeight = 80.f;
};

} // namespace deadaim