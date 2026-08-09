#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <cmath>

namespace deadaim {
namespace perspective {

constexpr float kDesignWidth = 1920.f;
constexpr float kDesignHeight = 1080.f;



constexpr float kHorizonY = 400.f;
constexpr float kVanishingX = kDesignWidth / 2.f;
constexpr float kFocalLength = 1200.f;   
constexpr float kCameraHeight = 0.967f;  

constexpr float kSpawnDepth = 18.f;
constexpr float kAttackDepth = 2.f;
constexpr float kZombieHeight = 0.6f;

constexpr float kSpawnLaneRatio = 0.611f;
constexpr float kArrivalLaneRatio = 0.15f;
constexpr float kConvergenceExponent = 1.6f;
constexpr float kAttackDepthSpread = 0.55f;

constexpr float kMuzzleX = 0.242f;
constexpr float kMuzzleY = 0.567f;
constexpr float kMuzzleZ = 1.0f;

// Past this depth a projectile is gone for good.
constexpr float kMaxProjectileDepth = 32.f;


inline float dot(sf::Vector3f a, sf::Vector3f b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float lengthSquared(sf::Vector3f v) {
    return dot(v, v);
}

inline sf::Vector3f normalized(sf::Vector3f v) {
    float len = std::sqrt(lengthSquared(v));
    return len > 1e-6f ? v / len : sf::Vector3f{0.f, 0.f, 1.f};
}

inline sf::Vector3f muzzleWorld() {
    return {kMuzzleX, kMuzzleY, kMuzzleZ};
}



inline sf::Vector2f project(sf::Vector3f p) {
    float scale = kFocalLength / p.z;
    return {kVanishingX + p.x * scale,
            kHorizonY + (kCameraHeight - p.y) * scale};
}


inline float projectScale(float z) {
    return kFocalLength / z;
}


inline float projectGroundY(float z) {
    return kHorizonY + (kCameraHeight * kFocalLength) / z;
}

inline float projectScreenX(float x, float z) {
    return kVanishingX + (kFocalLength * x) / z;
}


inline sf::Vector3f unproject(sf::Vector2f screen, float z) {
    float invScale = z / kFocalLength;
    return {(screen.x - kVanishingX) * invScale,
            kCameraHeight - (screen.y - kHorizonY) * invScale,
            z};
}

constexpr float kProjectileSpawnDepth = 1.0f;

// The camera itself, in world coordinates.
inline sf::Vector3f cameraWorld() {
    return {0.f, kCameraHeight, 0.f};
}

} 
} 