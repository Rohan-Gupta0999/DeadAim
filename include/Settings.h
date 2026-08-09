#pragma once

namespace deadaim {

struct Settings {
    float musicVolume = 35.f;
    float sfxVolume = 70.f;
    float sensitivity = 0.20f;

    static constexpr float kMinSensitivity = 0.08f;
    static constexpr float kMaxSensitivity = 0.45f;
};

} 