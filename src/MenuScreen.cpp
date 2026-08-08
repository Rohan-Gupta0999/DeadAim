#include "MenuScreen.h"
#include "AssetManager.h"
#include "Renderer.h"
#include "RenderLayer.h"
#include <iostream>

namespace deadaim {

namespace {
const sf::Color kButtonIdle(50, 50, 60);
const sf::Color kButtonHover(90, 90, 110);
const sf::Color kGold(240, 200, 80);

constexpr float kTitleY = 320.f;
constexpr float kSubtitleY = 440.f;
constexpr float kHighScoreY = 510.f;

// Centres a text object on its own middle. getLocalBounds() includes the
// glyphs' own offset, so both position and size are needed here --
// using size alone leaves text slightly off-centre.
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
    buildButton(m_quitButton, font, "QUIT", 730.f);

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
    if (!m_textReady) {
        return;
    }

    if (mode == Mode::MainMenu) {
        m_titleText->setString("DEADAIM");
        m_subtitleText->setString("Raise a finger gun to aim. Drop your thumb to fire.");
        m_subtitleText->setFillColor(sf::Color(200, 200, 200));

        // Nothing to brag about on a first run -- leave the line blank
        // rather than showing "HIGH SCORE 0".
        m_highScoreText->setString(
            info.highScore > 0 ? "HIGH SCORE  " + std::to_string(info.highScore) : "");
        m_highScoreText->setFillColor(kGold);

        m_primaryButton.label->setString("PLAY");
    } else {
        m_titleText->setString("GAME OVER");
        m_subtitleText->setString("FINAL SCORE  " + std::to_string(info.finalScore));
        m_subtitleText->setFillColor(sf::Color(200, 200, 200));

        if (info.isNewHighScore) {
            m_highScoreText->setString("NEW HIGH SCORE!");
        } else {
            m_highScoreText->setString("HIGH SCORE  " + std::to_string(info.highScore));
        }
        m_highScoreText->setFillColor(kGold);

        m_primaryButton.label->setString("RESTART");
    }

    // Re-centre everything whose string just changed -- a different word
    // is a different width, and each origin was computed for the old one.
    centreText(*m_titleText, kDesignWidth / 2.f, kTitleY);
    centreText(*m_subtitleText, kDesignWidth / 2.f, kSubtitleY);
    centreText(*m_highScoreText, kDesignWidth / 2.f, kHighScoreY);
    centreText(*m_primaryButton.label, kDesignWidth / 2.f,
               m_primaryButton.shape.getPosition().y);
}

MenuScreen::Action MenuScreen::update(sf::Vector2f mouseDesignPosition, bool clicked) {
    bool overPrimary = m_primaryButton.contains(mouseDesignPosition);
    bool overQuit = m_quitButton.contains(mouseDesignPosition);

    m_primaryButton.shape.setFillColor(overPrimary ? kButtonHover : kButtonIdle);
    m_quitButton.shape.setFillColor(overQuit ? kButtonHover : kButtonIdle);

    if (clicked && overPrimary) {
        return Action::StartGame;
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

    renderer.submit(m_primaryButton.shape, RenderLayer::UI);
    renderer.submit(m_quitButton.shape, RenderLayer::UI);

    if (m_primaryButton.label) {
        renderer.submit(*m_primaryButton.label, RenderLayer::UI);
    }
    if (m_quitButton.label) {
        renderer.submit(*m_quitButton.label, RenderLayer::UI);
    }
}

} // namespace deadaim