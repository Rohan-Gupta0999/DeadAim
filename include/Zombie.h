#pragma once

#include "IGameObject.h"
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector3.hpp>

namespace deadaim {

class Renderer;
class Player;
class AssetManager;

class Zombie : public IGameObject {
public:
    Zombie(float lane, Player& player, AssetManager& assets,
           int maxHealth, float speed);

    void update(float dt) override;
    void render(Renderer& renderer) const override;
    bool isAlive() const override;
    bool isTargetable() const;
    void takeDamage(int amount);
    sf::Vector3f getWorldPosition() const; 
    float getWorldRadius() const;          
    float getDepth() const;
    unsigned int getId() const;

private:
    sf::Sprite m_sprite;
    Player& m_player;
    int m_health;
    float m_speed;        
    float m_attackTimer = 0.f;

    float m_depth;              
    
    float m_screenY = 0.f;    
    float m_apparentHeight = 0.f;
    float m_baseHeight = 64.f;  
    float m_lane = 0.f;
    float m_attackDepth;
    float m_lateral = 0.f;
    unsigned int m_id;
    static unsigned int s_nextId;

    float m_hitFlashTimer = 0.f;
    float m_deathTimer = 0.f;
    float m_fallDirection = 1.f;

    bool m_dying = false;
    bool m_removalReady = false;

    static constexpr float kHitFlashSeconds = 0.09f;
    static constexpr float kDeathSeconds = 0.55f;
    static constexpr float kFallAngle = 82.f;

    static constexpr float kAttackIntervalSeconds = 1.f;
    static constexpr int kAttackDamage = 5;
    static constexpr float kBodyRadiusFraction = 0.30f;
};

} 