#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <cmath>

namespace deadaim {
namespace perspective {

constexpr float kDesignWidth = 1920.f;
constexpr float kDesignHeight = 1080.f;

// --- Camera model -----------------------------------------------------
// A pinhole camera. The player stands at the world origin looking down
// +z. x is lateral (0 = straight ahead), y is height above the ground,
// z is distance. Everything -- zombies, projectiles, explosions -- lives
// in these coordinates and is projected by the functions below. Nothing
// stores screen coordinates as its source of truth.

constexpr float kHorizonY = 400.f;
constexpr float kVanishingX = kDesignWidth / 2.f;
constexpr float kFocalLength = 1200.f;   // ~77 degree horizontal FOV at 1920 wide
constexpr float kCameraHeight = 0.967f;  // eye height in world units

constexpr float kSpawnDepth = 18.f;
constexpr float kAttackDepth = 2.f;
constexpr float kZombieHeight = 0.6f;

constexpr float kSpawnLaneRatio = 0.611f;
constexpr float kArrivalLaneRatio = 0.15f;
constexpr float kConvergenceExponent = 1.6f;
constexpr float kAttackDepthSpread = 0.55f;

// Where shots leave the weapon, in world space. Projects to roughly
// (1250, 880) on screen -- just inside the weapon sprite. Adjust these
// to line the tracer origin up with real art later.
constexpr float kMuzzleX = 0.242f;
constexpr float kMuzzleY = 0.567f;
constexpr float kMuzzleZ = 1.0f;

// Past this depth a projectile is gone for good.
constexpr float kMaxProjectileDepth = 32.f;

// --- Vector helpers ---------------------------------------------------
// Small free functions rather than a custom vector type: sf::Vector3f
// already gives us +, -, and scalar multiply.

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

// --- Projection -------------------------------------------------------

// The one true world-to-screen transform. Dividing by z is what produces
// correct size falloff, ground foreshortening, and the acceleration of
// anything approaching the camera.
inline sf::Vector2f project(sf::Vector3f p) {
    float scale = kFocalLength / p.z;
    return {kVanishingX + p.x * scale,
            kHorizonY + (kCameraHeight - p.y) * scale};
}

// Pixels per world unit at depth z. Also the shared render sort key:
// larger means nearer, so it orders zombies and projectiles together.
inline float projectScale(float z) {
    return kFocalLength / z;
}

// Convenience for ground-level points (y = 0), used by Environment and
// Zombie. Consistent with project({x, 0, z}) by construction.
inline float projectGroundY(float z) {
    return kHorizonY + (kCameraHeight * kFocalLength) / z;
}

inline float projectScreenX(float x, float z) {
    return kVanishingX + (kFocalLength * x) / z;
}

// The exact inverse of project(), for a chosen depth. This is what turns
// "the crosshair is at screen (x, y)" into a world point to aim at --
// substituting this back into project() returns the original screen
// point, which is why shots land where the crosshair is.
inline sf::Vector3f unproject(sf::Vector2f screen, float z) {
    float invScale = z / kFocalLength;
    return {(screen.x - kVanishingX) * invScale,
            kCameraHeight - (screen.y - kHorizonY) * invScale,
            z};
}

} // namespace perspective
} // namespace deadaim