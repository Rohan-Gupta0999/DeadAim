#include "Zombie.h"
#include "Player.h"
#include "AssetManager.h"
#include "Renderer.h"
#include "Perspective.h"
#include <algorithm>
#include <cmath>

namespace deadaim {


unsigned int Zombie::s_nextId = 1;

Zombie::Zombie(float lane, Player& player, AssetManager& assets,
               int maxHealth, float speed)
    : m_sprite(assets.getTexture("assets/textures/zombie.png"))
    , m_player(player)
    , m_health(maxHealth)
    , m_speed(speed)
    , m_depth(perspective::kSpawnDepth)
    , m_id(s_nextId++)
{
    sf::FloatRect bounds = m_sprite.getLocalBounds();
    m_baseHeight = std::max(bounds.size.y, 1.f);

    
    m_sprite.setOrigin({bounds.size.x / 2.f, bounds.size.y});

    
    lane = std::clamp(lane, -1.f, 1.f);
    m_lane = std::clamp(lane, -1.f, 1.f);

    m_attackDepth = perspective::kAttackDepth+ std::abs(m_lane) * perspective::kAttackDepthSpread;
    m_fallDirection = (m_lane >= 0.f) ? 1.f : -1.f;
    update(0.f); 
}

void Zombie::update(float dt) {
    if (m_dying) {
        m_deathTimer = std::max(0.f, m_deathTimer - dt);

        if (m_deathTimer <= 0.f) {
            m_removalReady = true;
        }

        float t = 1.f - (m_deathTimer / kDeathSeconds);

        m_sprite.setRotation(
            sf::degrees(kFallAngle * t * m_fallDirection)
        );

        auto alpha =
            static_cast<std::uint8_t>(255.f * (1.f - t * t));

        m_sprite.setColor(
            sf::Color(255, 255, 255, alpha)
        );

        return;
    }

    if (m_depth > m_attackDepth) {
        m_depth = std::max(
            m_depth - m_speed * dt,
            m_attackDepth
        );
    } else {
        m_attackTimer -= dt;

        if (m_attackTimer <= 0.f) {
            m_player.takeDamage(kAttackDamage);
            m_attackTimer = kAttackIntervalSeconds;
        }
    }

    float progress =
        (perspective::kSpawnDepth - m_depth)
        / (perspective::kSpawnDepth - m_attackDepth);

    progress = std::clamp(progress, 0.f, 1.f);

    float eased =
        std::pow(
            progress,
            perspective::kConvergenceExponent
        );

    float laneRatio =
        perspective::kSpawnLaneRatio
        + (perspective::kArrivalLaneRatio
        - perspective::kSpawnLaneRatio) * eased;

    m_lateral = m_lane * laneRatio * m_depth;

    m_screenY = perspective::projectGroundY(m_depth);

    float screenX =
        perspective::projectScreenX(
            m_lateral,
            m_depth
        );

    m_apparentHeight =
        perspective::kZombieHeight
        * perspective::projectScale(m_depth);

    float scale =
        m_apparentHeight / m_baseHeight;

    m_sprite.setScale({scale, scale});
    m_sprite.setPosition({screenX, m_screenY});

    m_hitFlashTimer =
        std::max(0.f, m_hitFlashTimer - dt);

    m_sprite.setColor(
        m_hitFlashTimer > 0.f
            ? sf::Color(255, 85, 85)
            : sf::Color::White
    );
}



bool Zombie::isAlive() const {
    return !m_removalReady;
}
bool Zombie::isTargetable() const {
    return m_health > 0;
}
void Zombie::takeDamage(int amount) {
    if (m_health <= 0) {
        return;
    }

    m_health = std::max(0, m_health - amount);
    m_hitFlashTimer = kHitFlashSeconds;

    if (m_health <= 0) {
        m_dying = true;
        m_deathTimer = kDeathSeconds;
    }
}
void Zombie::render(Renderer& renderer) const {
   
    renderer.submit(m_sprite, RenderLayer::World, perspective::projectScale(m_depth));
}
sf::Vector3f Zombie::getWorldPosition() const {
    return {m_lateral, perspective::kZombieHeight * 0.5f, m_depth};
}

float Zombie::getWorldRadius() const {
    return perspective::kZombieHeight * kBodyRadiusFraction;
}

float Zombie::getDepth() const {
    return m_depth;
}

unsigned int Zombie::getId() const {
    return m_id;
}

} 