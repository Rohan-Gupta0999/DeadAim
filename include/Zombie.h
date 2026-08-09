#pragma once

#include "IGameObject.h"
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector3.hpp>

namespace deadaim {

class Renderer;
class Player;
class AssetManager;

// Purpose: the single enemy type -- approaches through world space and is
// projected to screen by the shared camera model.
//
// Movement model: the zombie holds a world position (lateral, depth) and
// advances in depth at a constant world speed while steering laterally
// toward a narrow band in front of the player. Screen position, scale and
// draw order all fall out of projecting that world position -- no screen
// coordinates are ever interpolated directly.
//
// Constructed from a LANE in [-1, 1] rather than a pixel position, so
// WaveSystem never has to know about the projection.
class Zombie : public IGameObject {
public:
    Zombie(float lane, Player& player, AssetManager& assets,
           int maxHealth, float speed);

    void update(float dt) override;
    void render(Renderer& renderer) const override;
    bool isAlive() const override;

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

    static constexpr float kAttackIntervalSeconds = 1.f;
    static constexpr int kAttackDamage = 5;
    static constexpr float kBodyRadiusFraction = 0.30f; 
};

} 