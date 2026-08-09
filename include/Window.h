#pragma once

#include <SFML/Graphics.hpp>
#include <string>

namespace deadaim {

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

   
    bool consumeLeftClick();
    sf::Vector2i getLastClickPixelPosition() const;

private:
    sf::RenderWindow m_window;
    bool m_leftClickPending = false;
    sf::Vector2i m_lastClickPixel;

    static constexpr unsigned int kFramerateLimit = 60;
};

} 