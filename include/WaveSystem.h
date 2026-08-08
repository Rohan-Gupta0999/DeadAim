#pragma once

namespace deadaim {

class Scene;
class AssetManager;

class WaveSystem {
public:
    explicit WaveSystem(AssetManager& assets);

    void update(float dt, Scene& scene);
    void reset();
    int getCurrentWave() const;

private:
    int countLiveZombies(const Scene& scene) const;
    void spawnZombie(Scene& scene);

    static int zombiesInWave(int wave);
    static float spawnIntervalSeconds(int wave);
    static int zombieHealth(int wave);
    static float zombieSpeed(int wave);

    AssetManager& m_assets;
    int m_currentWave = 1;
    int m_zombiesRemainingToSpawn;
    float m_spawnTimer;
    float m_interWaveTimer = 0.f;
    bool m_waitingForNextWave = false;

    static constexpr float kInterWaveDelaySeconds = 3.f;
};

} 