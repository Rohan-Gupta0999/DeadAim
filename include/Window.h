#pragma once

#include <SFML/Graphics.hpp>
#include <string>

namespace deadaim {

// Purpose: owns the lifecycle of the game's OS window and the raw OS
// input that comes with it.
//
// Note on the two input styles: getMousePixelPosition/isLeftMouseButtonHeld
// poll live hardware state (good for continuous aim/fire). consumeLeftClick
// is event-driven and edge-triggered (good for menus, where one physical
// click must mean exactly one action).
class Window {
public:
    Window(unsigned int width, unsigned int height, const std::string& title);

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool isOpen() const;
    void close();
    void processEvents();
    void clear(const sf::Color& color = sf::Color::Black);
    void display();

    sf::RenderWindow& getNativeWindow();
    sf::Vector2i getMousePixelPosition() const;
    bool isLeftMouseButtonHeld() const;

    // Returns true at most once per click, then clears itself -- so a
    // single click can never be handled twice even if update() runs
    // multiple times in one frame.
    bool consumeLeftClick();
    sf::Vector2i getLastClickPixelPosition() const;

private:
    sf::RenderWindow m_window;
    bool m_leftClickPending = false;
    sf::Vector2i m_lastClickPixel;

    static constexpr unsigned int kFramerateLimit = 60;
};

} // namespace deadaim