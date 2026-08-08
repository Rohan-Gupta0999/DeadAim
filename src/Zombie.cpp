#include "Zombie.h"
#include "Player.h"
#include "AssetManager.h"
#include "Renderer.h"
#include "Perspective.h"
#include <algorithm>
#include <cmath>

namespace deadaim {

// Identity for piercing projectiles' hit lists.
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

    // Bottom-centre origin: a character stands ON the ground, so its feet
    // are its position. A centre origin would make a scaling zombie
    // appear to float upward as it approaches.
    m_sprite.setOrigin({bounds.size.x / 2.f, bounds.size.y});

    // The lane maps to both endpoints of a straight world-space path.
    // Same lane at both ends, different half-widths -- that ratio IS the V.
    lane = std::clamp(lane, -1.f, 1.f);
    m_lane = std::clamp(lane, -1.f, 1.f);

    m_attackDepth = perspective::kAttackDepth+ std::abs(m_lane) * perspective::kAttackDepthSpread;

    update(0.f); // project into place before the first frame draws
}

void Zombie::update(float dt) {
    if (m_depth > m_attackDepth) {
        // Constant WORLD speed. Screen speed accelerates on its own,
        // because projectGroundY divides by z -- exactly how approaching
        // motion looks in reality.
        m_depth = std::max(m_depth - m_speed * dt, m_attackDepth);
    } else {
        m_attackTimer -= dt;
        if (m_attackTimer <= 0.f) {
            m_player.takeDamage(kAttackDamage);
            m_attackTimer = kAttackIntervalSeconds;
        }
    }

    // 0 at spawn depth, 1 at attack depth.
    // 0 at spawn depth, 1 at this zombie's own attack depth.
    float progress = (perspective::kSpawnDepth - m_depth)
                   / (perspective::kSpawnDepth - m_attackDepth);
    progress = std::clamp(progress, 0.f, 1.f);

    // Interpolate the SCREEN OFFSET RATIO, then derive world lateral from
    // it. Because screen offset is focal * x/z, and x = ratio * z, the
    // offset is exactly focal * ratio -- so this curve directly controls
    // how the V narrows, instead of leaving it to emerge from the
    // division and stay stubbornly wide through the mid-range.
    float eased = std::pow(progress, perspective::kConvergenceExponent);
    float laneRatio = perspective::kSpawnLaneRatio
                    + (perspective::kArrivalLaneRatio - perspective::kSpawnLaneRatio) * eased;

    m_lateral = m_lane * laneRatio * m_depth;

    m_screenY = perspective::projectGroundY(m_depth);
    float screenX = perspective::projectScreenX(m_lateral, m_depth);

    m_apparentHeight = perspective::kZombieHeight * perspective::projectScale(m_depth);
    float scale = m_apparentHeight / m_baseHeight;

    m_sprite.setScale({scale, scale});
    m_sprite.setPosition({screenX, m_screenY});
}

void Zombie::render(Renderer& renderer) const {
    // Shared sort key with Projectile: focal/z, larger means nearer.
    // Linearly related to the projected ground Y this used to use, so
    // ordering among zombies is unchanged -- but now projectiles sort
    // into the same sequence.
    renderer.submit(m_sprite, RenderLayer::World, perspective::projectScale(m_depth));
}

bool Zombie::isAlive() const {
    return m_health > 0;
}

void Zombie::takeDamage(int amount) {
    m_health = std::max(0, m_health - amount);
}

sf::Vector3f Zombie::getWorldPosition() const {
    // Torso centre, not the feet: shots should connect with the body.
    return {m_lateral, perspective::kZombieHeight * 0.5f, m_depth};
}

float Zombie::getWorldRadius() const {
    // A fixed world size -- a distant zombie isn't physically smaller.
    // It's harder to hit because a small screen error becomes a large
    // world error at range, which falls out of the projection for free.
    return perspective::kZombieHeight * kBodyRadiusFraction;
}

float Zombie::getDepth() const {
    return m_depth;
}

unsigned int Zombie::getId() const {
    return m_id;
}

} // namespace deadaim