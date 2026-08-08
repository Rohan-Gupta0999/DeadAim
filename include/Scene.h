#pragma once

#include "IGameObject.h"
#include "Player.h"
#include <memory>
#include <vector>

namespace deadaim {

class Renderer;
class AudioSystem;

class Scene {
public:
    void addObject(std::unique_ptr<IGameObject> object);
    void reset();
    void update(float dt, sf::Vector2f aimTarget, bool fireHeld, AudioSystem& audio);
    void render(Renderer& renderer) const;

    Player& getPlayer();
    const Player& getPlayer() const;
    // True at most once per shot, then clears itself -- WeaponView reads
    // this to trigger recoil. Same consume pattern as Window's click.
    bool consumeShotFired();
    const std::vector<std::unique_ptr<IGameObject>>& getObjects() const;
    void addScore(int amount);
    int getScore() const;

private:
    std::vector<std::unique_ptr<IGameObject>> m_objects;
    Player m_player;
    int m_score = 0;
    bool m_shotFired = false;
};

} // namespace deadaim