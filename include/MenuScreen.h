#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include "Settings.h"

namespace deadaim {

class Renderer;
class AssetManager;


struct MenuInfo {
    int finalScore = 0;
    int highScore = 0;
    bool isNewHighScore = false;
};

class MenuScreen {
public:
    enum class Mode { MainMenu, GameOver , Settings , Paused };
    enum class Action { None, StartGame, Quit, OpenSettings, CloseSettings, SettingsChanged, Resume, ToMainMenu };

    explicit MenuScreen(AssetManager& assets);

    void setMode(Mode mode, const MenuInfo& info = {});
    Action update(sf::Vector2f mouseDesignPosition, bool clicked);
    void render(Renderer& renderer) const;
    void setSettings(const Settings& settings);
    const Settings& getSettings() const;
private:
    struct Button {
        sf::RectangleShape shape;
        std::optional<sf::Text> label;

        bool contains(sf::Vector2f point) const {
            return shape.getGlobalBounds().contains(point);
        }
    };
    struct Slider {
        sf::RectangleShape track;
        sf::RectangleShape fill;
        std::optional<sf::Text> label;
        float value = 0.f;
        float minValue = 0.f;
        float maxValue = 1.f;

        bool contains(sf::Vector2f point) const {
            return track.getGlobalBounds().contains(point);
        }
    };

    void buildSlider(Slider& slider, sf::Font* font, const std::string& name, float y);
    void refreshSlider(Slider& slider, const std::string& name);
    bool dragSlider(Slider& slider, sf::Vector2f mouse, const std::string& name);

    Mode m_mode = Mode::MainMenu;
    Button m_settingsButton;
    Slider m_musicSlider;
    Slider m_sfxSlider;
    Slider m_sensitivitySlider;
    Settings m_settings;

    static constexpr float kSliderWidth = 460.f;
    static constexpr float kSliderHeight = 26.f;
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

} 