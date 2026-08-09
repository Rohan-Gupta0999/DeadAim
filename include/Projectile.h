#pragma once

#include "IGameObject.h"
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector3.hpp>
#include <vector>
#include "IWeapon.h"

namespace deadaim {


struct ProjectileConfig {
    float speed = 40.f;                   
    int damage = 20;
    sf::Color color = sf::Color::Yellow;
    sf::Vector2f worldSize{0.06f, 0.022f};
    float collisionRadius = 0.06f;         
    int pierceCount = 1;
    float explosionRadius = 0.f;           
};

class Projectile : public IGameObject {
public:
    Projectile(const AimRay& ray, const ProjectileConfig& config);

    void update(float dt) override;
    void render(Renderer& renderer) const override;
    bool isAlive() const override;
    sf::Vector3f m_visualOffset;   
    float m_visualBlend = 1.f;     
    sf::Vector3f getWorldPosition() const;
    sf::Vector3f getPreviousWorldPosition() const;
    float getWorldRadius() const;
    int getDamage() const;
    float getExplosionRadius() const;

    bool hasHit(unsigned int zombieId) const;
    void registerHit(unsigned int zombieId);
    void consume();

private:
    void refreshVisual();

    sf::RectangleShape m_shape;
    sf::Vector3f m_position;
    sf::Vector3f m_previousPosition;
    sf::Vector3f m_velocity;
    sf::Vector2f m_worldSize;
    int m_damage;
    float m_worldRadius;
    float m_explosionRadius;
    int m_piercesRemaining;
    bool m_expired = false;
    std::vector<unsigned int> m_alreadyHit;

    
    static constexpr float kMinScreenLength = 3.f;
    static constexpr float kMinScreenThickness = 2.f;
    static constexpr float kMaxScreenLength = 160.f;
    static constexpr float kMaxScreenThickness = 90.f;
   
    static constexpr float kVisualBlendSeconds = 0.10f;
};

} 