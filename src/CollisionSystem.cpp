#include "CollisionSystem.h"
#include "Scene.h"
#include "Projectile.h"
#include "Zombie.h"
#include "Explosion.h"
#include "AudioSystem.h"
#include "Perspective.h"
#include <algorithm>
#include <memory>
#include <vector>

namespace deadaim {

namespace {
constexpr int kScorePerKill = 100;

bool spheresOverlap(sf::Vector3f a, float radiusA, sf::Vector3f b, float radiusB) {
    float radiusSum = radiusA + radiusB;
    return perspective::lengthSquared(a - b) < radiusSum * radiusSum;
}

bool sweptSphereHit(sf::Vector3f from, sf::Vector3f to, float movingRadius,
                     sf::Vector3f target, float targetRadius) {
    sf::Vector3f segment = to - from;
    float segmentLengthSquared = perspective::lengthSquared(segment);
    float radiusSum = movingRadius + targetRadius;

    if (segmentLengthSquared < 1e-8f) {
        return spheresOverlap(to, movingRadius, target, targetRadius);
    }

    float t = perspective::dot(target - from, segment) / segmentLengthSquared;
    t = std::clamp(t, 0.f, 1.f);

    sf::Vector3f closest = from + segment * t;
    return perspective::lengthSquared(target - closest) < radiusSum * radiusSum;
}

void damageZombie(Scene& scene, Zombie& zombie, int amount) {
    bool wasTargetable = zombie.isTargetable();
    zombie.takeDamage(amount);

    if (wasTargetable && !zombie.isTargetable()) {
        scene.addScore(kScorePerKill);
    }
}
} // namespace

void CollisionSystem::update(Scene& scene, AudioSystem& audio) {
    std::vector<Projectile*> projectiles;
    std::vector<Zombie*> zombies;

    for (const auto& object : scene.getObjects()) {
        if (auto* projectile = dynamic_cast<Projectile*>(object.get())) {
            projectiles.push_back(projectile);
        } else if (auto* zombie = dynamic_cast<Zombie*>(object.get())) {
            zombies.push_back(zombie);
        }
    }

    for (Projectile* projectile : projectiles) {
        if (!projectile->isAlive()) {
            continue;
        }

        for (Zombie* zombie : zombies) {
            if (!zombie->isTargetable()) {
                 continue;
            }
            // Already damaged this one -- a piercing arrow stays within
            // range of a zombie for several consecutive ticks.
            if (projectile->hasHit(zombie->getId())) {
                continue;
            }

            if (!sweptSphereHit(projectile->getPreviousWorldPosition(),
                                 projectile->getWorldPosition(),
                                 projectile->getWorldRadius(),
                                 zombie->getWorldPosition(),
                                 zombie->getWorldRadius())) {
                continue;
            }

            sf::Vector3f impactPoint = projectile->getWorldPosition();
            damageZombie(scene, *zombie, projectile->getDamage());
            projectile->registerHit(zombie->getId()); // spends one pierce

            float blastRadius = projectile->getExplosionRadius();
            if (blastRadius > 0.f) {
                for (Zombie* other : zombies) {
                    if (other == zombie || !other->isTargetable()) {
                        continue;
                    }
                    if (spheresOverlap(impactPoint, blastRadius,
                                        other->getWorldPosition(), other->getWorldRadius())) {
                        damageZombie(scene, *other, projectile->getDamage());
                    }
                }

                audio.play(SoundId::Explosion);
               
                scene.addObject(std::make_unique<Explosion>(impactPoint, blastRadius));
                projectile->consume();
            }

            if (!projectile->isAlive()) {
                break; 
            }
        }
    }
}

} 