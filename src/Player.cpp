#include "Player.h"
#include "Renderer.h"
#include "IWeapon.h"
#include "Gun.h"
#include <algorithm>
#include "Projectile.h"
#include "Bow.h"
#include "Fireball.h"
#include "Perspective.h"
#include "Settings.h"

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

    m_crosshairPosition += (aimTarget - m_crosshairPosition) * m_crosshairSmoothing;
    m_crosshairShape.setPosition(m_crosshairPosition);

    m_equippedWeapon->update(dt);
}
void Player::setSensitivity(float sensitivity) {
    m_crosshairSmoothing = std::clamp(sensitivity,
                                       Settings::kMinSensitivity,
                                       Settings::kMaxSensitivity);
}
void Player::reset() {
    m_health = kMaxHealth;
    m_crosshairPosition = {kDesignWidth / 2.f, kDesignHeight / 2.f};
    m_crosshairShape.setPosition(m_crosshairPosition);
    
    m_equippedType = WeaponType::Gun;
    m_equippedWeapon = std::make_unique<Gun>();
}

void Player::equipWeapon(WeaponType type) {
    
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
    AimRay ray;

    ray.origin = perspective::unproject(m_crosshairPosition,
                                        perspective::kProjectileSpawnDepth);

    ray.direction = perspective::normalized(ray.origin - perspective::cameraWorld());

    ray.visualOrigin = perspective::muzzleWorld();

    return m_equippedWeapon->tryFire(ray);
}

}