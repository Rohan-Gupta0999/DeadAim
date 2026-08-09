#include "MenuScreen.h"
#include "AssetManager.h"
#include "Renderer.h"
#include "RenderLayer.h"
#include <iostream>
#include <algorithm>

namespace deadaim {

namespace {
const sf::Color kButtonIdle(50, 50, 60);
const sf::Color kButtonHover(90, 90, 110);
const sf::Color kGold(240, 200, 80);

constexpr float kTitleY = 320.f;
constexpr float kSubtitleY = 440.f;
constexpr float kHighScoreY = 510.f;
constexpr float kSliderLabelOffset = 34.f;

void centreText(sf::Text& text, float x, float y) {
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                    bounds.position.y + bounds.size.y / 2.f});
    text.setPosition({x, y});
}
} // namespace

MenuScreen::MenuScreen(AssetManager& assets) {
    m_backdrop.setSize({kDesignWidth, kDesignHeight});
    m_backdrop.setPosition({0.f, 0.f});
    m_backdrop.setFillColor(sf::Color(10, 10, 15, 210)); // dims the frozen game behind it

    sf::Font* font = assets.getFont("assets/fonts/main.ttf");
    if (font == nullptr) {
        std::cerr << "[MenuScreen] No font found -- menu buttons will still "
                     "work but have no labels.\n";
    }

    buildButton(m_primaryButton, font, "PLAY", 620.f);
    buildButton(m_quitButton, font, "QUIT", 840.f);
    buildButton(m_settingsButton, font, "SETTINGS", 730.f);

    buildSlider(m_musicSlider, font, "MUSIC", 500.f);
    m_musicSlider.minValue = 0.f;
    m_musicSlider.maxValue = 100.f;

    buildSlider(m_sfxSlider, font, "EFFECTS", 620.f);
    m_sfxSlider.minValue = 0.f;
    m_sfxSlider.maxValue = 100.f;

    buildSlider(m_sensitivitySlider, font, "SENSITIVITY", 740.f);
    m_sensitivitySlider.minValue = Settings::kMinSensitivity;
    m_sensitivitySlider.maxValue = Settings::kMaxSensitivity;
    if (font != nullptr) {
        m_titleText.emplace(*font, "DEADAIM", 96);
        m_titleText->setFillColor(sf::Color(220, 60, 60));
        centreText(*m_titleText, kDesignWidth / 2.f, kTitleY);

        m_subtitleText.emplace(*font, "", 34);
        m_subtitleText->setFillColor(sf::Color(200, 200, 200));
        centreText(*m_subtitleText, kDesignWidth / 2.f, kSubtitleY);

        m_highScoreText.emplace(*font, "", 30);
        m_highScoreText->setFillColor(kGold);
        centreText(*m_highScoreText, kDesignWidth / 2.f, kHighScoreY);

        m_textReady = true;
    }
}

void MenuScreen::buildButton(Button& button, sf::Font* font,
                              const std::string& text, float y) {
    button.shape.setSize({kButtonWidth, kButtonHeight});
    button.shape.setOrigin({kButtonWidth / 2.f, kButtonHeight / 2.f});
    button.shape.setPosition({kDesignWidth / 2.f, y});
    button.shape.setFillColor(kButtonIdle);
    button.shape.setOutlineColor(sf::Color(160, 160, 170));
    button.shape.setOutlineThickness(2.f);

    if (font != nullptr) {
        button.label.emplace(*font, text, 40);
        button.label->setFillColor(sf::Color::White);
        centreText(*button.label, kDesignWidth / 2.f, y);
    }
}

void MenuScreen::setMode(Mode mode, const MenuInfo& info) {
    m_mode = mode;
    if (!m_textReady) {
        return;
    }

    if (mode == Mode::MainMenu) {
        m_titleText->setString("DEADAIM");
        m_subtitleText->setString("Raise a finger gun to aim. Drop your thumb to fire.");
        m_subtitleText->setFillColor(sf::Color(200, 200, 200));
        m_highScoreText->setString(
            info.highScore > 0 ? "HIGH SCORE  " + std::to_string(info.highScore) : "");
        m_primaryButton.label->setString("PLAY");
        m_quitButton.label->setString("QUIT");
    } else if (mode == Mode::GameOver) {
        m_titleText->setString("GAME OVER");
        m_subtitleText->setString("FINAL SCORE  " + std::to_string(info.finalScore));
        m_subtitleText->setFillColor(sf::Color(200, 200, 200));
        m_highScoreText->setString(info.isNewHighScore
            ? "NEW HIGH SCORE!"
            : "HIGH SCORE  " + std::to_string(info.highScore));
        m_primaryButton.label->setString("RESTART");
        m_quitButton.label->setString("QUIT");
    } else {
        m_titleText->setString("SETTINGS");
        m_subtitleText->setString("");
        m_highScoreText->setString("");
        m_quitButton.label->setString("BACK");
    }

    m_highScoreText->setFillColor(kGold);
    centreText(*m_titleText, kDesignWidth / 2.f, kTitleY);
    centreText(*m_subtitleText, kDesignWidth / 2.f, kSubtitleY);
    centreText(*m_highScoreText, kDesignWidth / 2.f, kHighScoreY);
    centreText(*m_primaryButton.label, kDesignWidth / 2.f,
               m_primaryButton.shape.getPosition().y);
    centreText(*m_quitButton.label, kDesignWidth / 2.f,
               m_quitButton.shape.getPosition().y);
}

MenuScreen::Action MenuScreen::update(sf::Vector2f mouseDesignPosition, bool clicked) {
    if (m_mode == Mode::Settings) {
        bool overQuit = m_quitButton.contains(mouseDesignPosition);
        m_quitButton.shape.setFillColor(overQuit ? kButtonHover : kButtonIdle);

        if (clicked) {
            bool changed = false;
            changed |= dragSlider(m_musicSlider, mouseDesignPosition, "MUSIC");
            changed |= dragSlider(m_sfxSlider, mouseDesignPosition, "EFFECTS");
            changed |= dragSlider(m_sensitivitySlider, mouseDesignPosition, "SENSITIVITY");

            if (changed) {
                m_settings.musicVolume = m_musicSlider.value;
                m_settings.sfxVolume = m_sfxSlider.value;
                m_settings.sensitivity = m_sensitivitySlider.value;
                return Action::SettingsChanged;
            }
            if (overQuit) {
                return Action::CloseSettings;
            }
        }
        return Action::None;
    }

    bool overPrimary = m_primaryButton.contains(mouseDesignPosition);
    bool overQuit = m_quitButton.contains(mouseDesignPosition);
    bool overSettings = m_settingsButton.contains(mouseDesignPosition);

    m_primaryButton.shape.setFillColor(overPrimary ? kButtonHover : kButtonIdle);
    m_quitButton.shape.setFillColor(overQuit ? kButtonHover : kButtonIdle);
    m_settingsButton.shape.setFillColor(overSettings ? kButtonHover : kButtonIdle);

    if (clicked && overPrimary) {
        return Action::StartGame;
    }
    if (clicked && overSettings) {
        return Action::OpenSettings;
    }
    if (clicked && overQuit) {
        return Action::Quit;
    }
    return Action::None;
}

void MenuScreen::render(Renderer& renderer) const {
    renderer.submit(m_backdrop, RenderLayer::UI);

    if (m_textReady) {
        renderer.submit(*m_titleText, RenderLayer::UI);
        renderer.submit(*m_subtitleText, RenderLayer::UI);
        renderer.submit(*m_highScoreText, RenderLayer::UI);
    }

    if (m_mode == Mode::Settings) {
        for (const Slider* slider : {&m_musicSlider, &m_sfxSlider, &m_sensitivitySlider}) {
            renderer.submit(slider->track, RenderLayer::UI);
            renderer.submit(slider->fill, RenderLayer::UI);
            if (slider->label) {
                renderer.submit(*slider->label, RenderLayer::UI);
            }
        }
        renderer.submit(m_quitButton.shape, RenderLayer::UI);
        if (m_quitButton.label) {
            renderer.submit(*m_quitButton.label, RenderLayer::UI);
        }
        return;
    }

    renderer.submit(m_primaryButton.shape, RenderLayer::UI);
    renderer.submit(m_quitButton.shape, RenderLayer::UI);
    if (m_primaryButton.label) {
        renderer.submit(*m_primaryButton.label, RenderLayer::UI);
    }
    if (m_quitButton.label) {
        renderer.submit(*m_quitButton.label, RenderLayer::UI);
    }

    if (m_mode == Mode::MainMenu) {
        renderer.submit(m_settingsButton.shape, RenderLayer::UI);
        if (m_settingsButton.label) {
            renderer.submit(*m_settingsButton.label, RenderLayer::UI);
        }
    }
}
void MenuScreen::buildSlider(Slider& slider, sf::Font* font,
                              const std::string& name, float y) {
    slider.track.setSize({kSliderWidth, kSliderHeight});
    slider.track.setOrigin({kSliderWidth / 2.f, kSliderHeight / 2.f});
    slider.track.setPosition({kDesignWidth / 2.f, y});
    slider.track.setFillColor(sf::Color(45, 45, 55));
    slider.track.setOutlineColor(sf::Color(150, 150, 165));
    slider.track.setOutlineThickness(2.f);

    slider.fill.setSize({kSliderWidth, kSliderHeight});
    slider.fill.setOrigin({kSliderWidth / 2.f, kSliderHeight / 2.f});
    slider.fill.setPosition({kDesignWidth / 2.f, y});
    slider.fill.setFillColor(sf::Color(120, 170, 220));

    if (font != nullptr) {
        slider.label.emplace(*font, name, 26);
        slider.label->setFillColor(sf::Color(210, 210, 210));
        centreText(*slider.label, kDesignWidth / 2.f, y - kSliderLabelOffset);
    }
}

void MenuScreen::refreshSlider(Slider& slider, const std::string& name) {
    float span = slider.maxValue - slider.minValue;
    float fraction = (span > 0.0001f) ? (slider.value - slider.minValue) / span : 0.f;
    fraction = std::clamp(fraction, 0.f, 1.f);

    float width = kSliderWidth * fraction;
    slider.fill.setSize({width, kSliderHeight});
    slider.fill.setOrigin({kSliderWidth / 2.f, kSliderHeight / 2.f});

    if (slider.label) {
        int percent = static_cast<int>(fraction * 100.f + 0.5f);
        slider.label->setString(name + "   " + std::to_string(percent) + "%");
        centreText(*slider.label, kDesignWidth / 2.f,
                   slider.track.getPosition().y - kSliderLabelOffset);
    }
}

bool MenuScreen::dragSlider(Slider& slider, sf::Vector2f mouse, const std::string& name) {
    if (!slider.contains(mouse)) {
        return false;
    }

    float left = slider.track.getPosition().x - kSliderWidth / 2.f;
    float fraction = std::clamp((mouse.x - left) / kSliderWidth, 0.f, 1.f);
    slider.value = slider.minValue + fraction * (slider.maxValue - slider.minValue);
    refreshSlider(slider, name);
    return true;
}

void MenuScreen::setSettings(const Settings& settings) {
    m_settings = settings;
    m_musicSlider.value = settings.musicVolume;
    m_sfxSlider.value = settings.sfxVolume;
    m_sensitivitySlider.value = settings.sensitivity;
    refreshSlider(m_musicSlider, "MUSIC");
    refreshSlider(m_sfxSlider, "EFFECTS");
    refreshSlider(m_sensitivitySlider, "SENSITIVITY");
}

const Settings& MenuScreen::getSettings() const {
    return m_settings;
}
} 