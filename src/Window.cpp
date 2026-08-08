#include "Window.h"

namespace deadaim {

Window::Window(unsigned int width, unsigned int height, const std::string& title)
    : m_window(sf::VideoMode({width, height}), title)
{
    // Do NOT also call setVerticalSyncEnabled -- SFML's docs are explicit
    // that combining the two makes frame pacing worse, not better.
    m_window.setFramerateLimit(kFramerateLimit);
}

bool Window::isOpen() const {
    return m_window.isOpen();
}

void Window::close() {
    m_window.close();
}

void Window::processEvents() {
    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
        } else if (const auto* pressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (pressed->button == sf::Mouse::Button::Left) {
                m_leftClickPending = true;
                m_lastClickPixel = pressed->position;
            }
        }
    }
}

void Window::clear(const sf::Color& color) {
    m_window.clear(color);
}

void Window::display() {
    m_window.display();
}

sf::RenderWindow& Window::getNativeWindow() {
    return m_window;
}

sf::Vector2i Window::getMousePixelPosition() const {
    return sf::Mouse::getPosition(m_window);
}

bool Window::isLeftMouseButtonHeld() const {
    return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
}

bool Window::consumeLeftClick() {
    bool wasPending = m_leftClickPending;
    m_leftClickPending = false;
    return wasPending;
}

sf::Vector2i Window::getLastClickPixelPosition() const {
    return m_lastClickPixel;
}

} // namespace deadaim