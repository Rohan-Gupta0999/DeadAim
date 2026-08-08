#pragma once

#include <SFML/System/Vector2.hpp>
#include <memory>
#include <SFML/System/Vector3.hpp>

namespace deadaim {

class Projectile;

// Which weapon is equipped. Lives here rather than in Player so that
// Application can request a switch without knowing the concrete classes.
enum class WeaponType {
    Gun,
    Bow,
    Fireball
};

class IWeapon {
public:
    virtual ~IWeapon() = default;

    virtual void update(float dt) = 0;

    // Returns nullptr if the weapon can't fire right now (cooldown,
    // empty magazine, mid-reload) -- an ordinary outcome, not an error.
    // Both arguments are WORLD space: where the shot leaves the weapon,
    // and a unit direction into the scene. Player builds these by
    // unprojecting the crosshair -- weapons never touch screen space.
    virtual std::unique_ptr<Projectile> tryFire(sf::Vector3f muzzleWorld,
                                                 sf::Vector3f directionWorld) = 0;
    virtual const char* getName() const = 0;
};

} // namespace deadaim