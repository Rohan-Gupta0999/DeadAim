#pragma once

#include <SFML/System/Vector2.hpp>
#include <memory>
#include <SFML/System/Vector3.hpp>

namespace deadaim {

class Projectile;


struct AimRay {
    sf::Vector3f origin;      
    sf::Vector3f direction;    
    sf::Vector3f visualOrigin; 
};

enum class WeaponType {
    Gun,
    Bow,
    Fireball
};

class IWeapon {
public:
    virtual ~IWeapon() = default;

    virtual void update(float dt) = 0;

    virtual std::unique_ptr<Projectile> tryFire(const AimRay& ray) = 0;
    virtual const char* getName() const = 0;
};

} 