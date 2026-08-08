#pragma once

#include "RenderLayer.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include "IWeapon.h" 

namespace deadaim {

class Renderer;
class IWeapon;
class Projectile;

class Player {
public:
    Player();
    ~Player();

    void update(float dt, sf::Vector2f aimTarget);
    void render(Renderer& renderer) const;
    void reset();
    void takeDamage(int amount);
    bool isAlive() const;
    int getHealth() const;
    int getMaxHealth() const;
    const char* getWeaponName() const;

    sf::Vector2f getCrosshairPosition() const;
    sf::Vector2f getPosition() const;
    std::unique_ptr<Projectile> tryFire();

    void equipWeapon(WeaponType type);
    WeaponType getEquippedWeapon() const;

private:
    sf::Vector2f m_crosshairPosition;
    sf::CircleShape m_crosshairShape;
    int m_health;
    std::unique_ptr<IWeapon> m_equippedWeapon;
    WeaponType m_equippedType = WeaponType::Gun;

    static constexpr int kMaxHealth = 100;
    static constexpr float kCrosshairSmoothing = 0.2f;
    static constexpr float kCrosshairRadius = 14.f;
    static constexpr float kDesignWidth = 1920.f;
    static constexpr float kDesignHeight = 1080.f;

    static const sf::Vector2f kMuzzlePosition;
};

} // namespace deadaim