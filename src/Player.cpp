#include "Player.h"
#include "Renderer.h"
#include "IWeapon.h"
#include "Gun.h"
#include <algorithm>
#include "Projectile.h"
#include "Bow.h"
#include "Fireball.h"
#include "Perspective.h"

namespace deadaim {

const sf::Vector2f Player::kMuzzlePosition{960.f, 1000.f};

Player::Player()
    : m_crosshairPosition(kDesignWidth / 2.f, kDesignHeight / 2.f)
    , m_crosshairShape(kCrosshairRadius)
    , m_health(kMaxHealth)
    , m_equippedWeapon(std::make_unique<Gun>())
{
    m_crosshairShape.setOrigin({kCrosshairRadius, kCrosshairRadius});
    m_crosshairShape.setFillColor(sf::Color::Transparent);
    m_crosshairShape.setOutlineColor(sf::Color::White);
    m_crosshairShape.setOutlineThickness(2.f);
}

Player::~Player() = default;

void Player::update(float dt, sf::Vector2f aimTarget) {
    aimTarget.x = std::clamp(aimTarget.x, 0.f, kDesignWidth);
    aimTarget.y = std::clamp(aimTarget.y, 0.f, kDesignHeight);

    m_crosshairPosition += (aimTarget - m_crosshairPosition) * kCrosshairSmoothing;
    m_crosshairShape.setPosition(m_crosshairPosition);

    m_equippedWeapon->update(dt);
}

void Player::reset() {
    m_health = kMaxHealth;
    m_crosshairPosition = {kDesignWidth / 2.f, kDesignHeight / 2.f};
    m_crosshairShape.setPosition(m_crosshairPosition);
    // Fresh weapon: also clears a half-empty magazine or an in-progress
    // reload left over from the previous run.
    m_equippedType = WeaponType::Gun;
    m_equippedWeapon = std::make_unique<Gun>();
}

void Player::equipWeapon(WeaponType type) {
    // Critical: bail out if it's already equipped. Application calls this
    // every frame the gesture is held -- rebuilding the weapon each time
    // would reset its cooldown continuously and it could never fire.
    if (type == m_equippedType) {
        return;
    }

    m_equippedType = type;
    switch (type) {
    case WeaponType::Gun: m_equippedWeapon = std::make_unique<Gun>(); break;
    case WeaponType::Bow: m_equippedWeapon = std::make_unique<Bow>(); break;
    case WeaponType::Fireball: m_equippedWeapon = std::make_unique<Fireball>(); break;
    }
}

WeaponType Player::getEquippedWeapon() const {
    return m_equippedType;
}

void Player::render(Renderer& renderer) const {
    renderer.submit(m_crosshairShape, RenderLayer::UI);
}

void Player::takeDamage(int amount) {
    m_health = std::max(0, m_health - amount);
}

bool Player::isAlive() const {
    return m_health > 0;
}

int Player::getHealth() const {
    return m_health;
}

int Player::getMaxHealth() const {
    return kMaxHealth;
}

const char* Player::getWeaponName() const {
    return m_equippedWeapon->getName();
}

sf::Vector2f Player::getCrosshairPosition() const {
    return m_crosshairPosition;
}

sf::Vector2f Player::getPosition() const {
    return kMuzzlePosition;
}

std::unique_ptr<Projectile> Player::tryFire() {
    // Build the world-space aim ray. Unprojecting the crosshair at spawn
    // depth gives the world point the crosshair is "over"; the direction
    // is that point minus the muzzle.
    //
    // Because the muzzle sits right of the camera, the path converges on
    // the crosshair exactly at spawn depth and reads slightly right of
    // it up close -- real weapon parallax, the same convergence real FPS
    // games call the zero range.
    sf::Vector3f muzzle = perspective::muzzleWorld();
    sf::Vector3f aimPoint = perspective::unproject(m_crosshairPosition,
                                                    perspective::kSpawnDepth);
    sf::Vector3f direction = perspective::normalized(aimPoint - muzzle);

    return m_equippedWeapon->tryFire(muzzle, direction);
}

} // namespace deadaim