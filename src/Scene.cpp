#include "Scene.h"
#include "Projectile.h" // needed: converting unique_ptr<Projectile> to
                         // unique_ptr<IGameObject> requires the complete type
#include <algorithm>
#include "AudioSystem.h"
#include "IWeapon.h"   // for WeaponType

namespace deadaim {

void Scene::addObject(std::unique_ptr<IGameObject> object) {
    m_objects.push_back(std::move(object));
}

void Scene::update(float dt, sf::Vector2f aimTarget, bool fireHeld, AudioSystem& audio) {
    m_player.update(dt, aimTarget);

    if (fireHeld) {
        // Only sounds when a shot is ACTUALLY produced -- tryFire returns
        // null on cooldown, empty magazine, or mid-reload, and a click
        // with no bullet would be misleading feedback.
        if (auto projectile = m_player.tryFire()) {
            switch (m_player.getEquippedWeapon()) {
            case WeaponType::Gun:      audio.play(SoundId::GunFire); break;
            case WeaponType::Bow:      audio.play(SoundId::BowFire); break;
            case WeaponType::Fireball: audio.play(SoundId::FireballFire); break;
            }
            m_shotFired = true;
            addObject(std::move(projectile));
        }
    }

    for (auto& object : m_objects) {
        object->update(dt);
    }

    m_objects.erase(
        std::remove_if(m_objects.begin(), m_objects.end(),
            [](const std::unique_ptr<IGameObject>& object) {
                return !object->isAlive();
            }),
        m_objects.end());
}

void Scene::render(Renderer& renderer) const {
    m_player.render(renderer);

    for (const auto& object : m_objects) {
        object->render(renderer);
    }
}

void Scene::reset() {
    // Objects are cleared BEFORE the player is reset. Every Zombie holds
    // a Player& -- clearing first guarantees nothing is left holding a
    // reference while the player's state changes underneath it.
    m_objects.clear();
    m_player.reset();
    m_score = 0;
}

Player& Scene::getPlayer() {
    return m_player;
}

const Player& Scene::getPlayer() const {
    return m_player;
}

const std::vector<std::unique_ptr<IGameObject>>& Scene::getObjects() const {
    return m_objects;
}

void Scene::addScore(int amount) {
    m_score += amount;
}

int Scene::getScore() const {
    return m_score;
}

bool Scene::consumeShotFired() {
    bool fired = m_shotFired;
    m_shotFired = false;
    return fired;
}

} // namespace deadaim