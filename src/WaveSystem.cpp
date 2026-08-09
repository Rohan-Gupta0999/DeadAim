#include "WaveSystem.h"
#include "Scene.h"
#include "Zombie.h"
#include <algorithm>
#include <memory>
#include <random>

namespace deadaim {

namespace {
std::mt19937& rng() {
    static std::mt19937 generator(std::random_device{}());
    return generator;
}
} 

constexpr float kSpawnY = 415.f;

constexpr float kSpawnMinX = 300.f;
constexpr float kSpawnMaxX = 1620.f;


WaveSystem::WaveSystem(AssetManager& assets)
    : m_assets(assets)
    , m_zombiesRemainingToSpawn(zombiesInWave(m_currentWave))
    , m_spawnTimer(spawnIntervalSeconds(m_currentWave))
{
}

void WaveSystem::update(float dt, Scene& scene) {
    if (m_waitingForNextWave) {
        m_interWaveTimer -= dt;
        if (m_interWaveTimer <= 0.f) {
            ++m_currentWave;
            m_zombiesRemainingToSpawn = zombiesInWave(m_currentWave);
            m_spawnTimer = spawnIntervalSeconds(m_currentWave); 
            m_waitingForNextWave = false;
        }
        return;
    }

    if (m_zombiesRemainingToSpawn > 0) {
        m_spawnTimer -= dt;
        if (m_spawnTimer <= 0.f) {
            spawnZombie(scene);
            --m_zombiesRemainingToSpawn;
            m_spawnTimer = spawnIntervalSeconds(m_currentWave);
        }
        return;
    }

   
    if (countLiveZombies(scene) == 0) {
        m_waitingForNextWave = true;
        m_interWaveTimer = kInterWaveDelaySeconds;
    }
}

int WaveSystem::getCurrentWave() const {
    return m_currentWave;
}

int WaveSystem::countLiveZombies(const Scene& scene) const {
    int count = 0;

    for (const auto& object : scene.getObjects()) {
        auto* zombie = dynamic_cast<Zombie*>(object.get());

        if (zombie != nullptr && zombie->isTargetable()) {
            ++count;
        }
    }

    return count;
}

void WaveSystem::spawnZombie(Scene& scene) {
    
    std::uniform_real_distribution<float> laneDistribution(-1.f, 1.f);

    scene.addObject(std::make_unique<Zombie>(
        laneDistribution(rng()), scene.getPlayer(), m_assets,
        zombieHealth(m_currentWave), zombieSpeed(m_currentWave)));
}

void WaveSystem::reset() {
    m_currentWave = 1;
    m_zombiesRemainingToSpawn = zombiesInWave(m_currentWave);
    m_spawnTimer = spawnIntervalSeconds(m_currentWave);
    m_interWaveTimer = 0.f;
    m_waitingForNextWave = false;
}

int WaveSystem::zombiesInWave(int wave) {
    return std::min(5 + (wave - 1) * 2, 30);
}

float WaveSystem::spawnIntervalSeconds(int wave) {
    return std::max(2.f - (wave - 1) * 0.1f, 0.5f);
}

int WaveSystem::zombieHealth(int wave) {
    return std::min(60 + (wave - 1) * 8, 200);
}

float WaveSystem::zombieSpeed(int wave) {
    
    return std::min(1.45f + (wave - 1) * 0.07f, 3.0f);
}

} 