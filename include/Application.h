#pragma once

#include "Window.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "Scene.h"
#include "CollisionSystem.h"
#include "WaveSystem.h"
#include "VisionClient.h"
#include "HUD.h"
#include "MenuScreen.h"
#include "SaveSystem.h"
#include "AudioSystem.h"
#include "Environment.h"
#include "WeaponView.h"

namespace deadaim {

enum class GameState {
    MainMenu,
    Playing,
    GameOver
};

class Application {
public:
    Application();

    void run();

private:
    void update(float dt);
    void updatePlaying(float dt, sf::Vector2f mouseDesignPosition);
    void render();
    void resetGame();

    Window m_window;
    Renderer m_renderer;
    AssetManager m_assetManager; // MUST stay before m_hud and m_menu: sf::Text
                                 // holds a font reference, and members destruct
                                 // in reverse declaration order.
    Scene m_scene;
    CollisionSystem m_collisionSystem;
    WaveSystem m_waveSystem;
    VisionClient m_visionClient;
    HUD m_hud;
    MenuScreen m_menu;
    SaveSystem m_saveSystem;
    GameState m_state = GameState::MainMenu;
    AudioSystem m_audio;
    Environment m_environment;
    WeaponView m_weaponView;
    static constexpr float kFixedTimestep = 1.0f / 60.0f;
    // What the player is asking for this tick, resolved from gesture and
    // mouse input. Extracted so updatePlaying() reads as "get intent,
    // run simulation" rather than interleaving the two.
    struct PlayerIntent {
        sf::Vector2f aimTarget;
        bool shootRequest = false;
        bool visionTracking = false;
        GestureType gesture = GestureType::None;
        float fireballCharge = 0.f;
        // Previous tick's values. Health dropping IS the player being hit;
        // score rising IS a zombie dying -- so these two deltas give us both
        // sounds without threading audio through Zombie or CollisionSystem.
        
    };

    PlayerIntent resolveInput(float dt, sf::Vector2f mouseDesignPosition);
    int m_lastHealth = 100;
    int m_lastScore = 0;
    bool m_holdWasActive = false;   // previous tick's hold state (bow draw / fireball charge)
    bool m_visionWasTracking = false;
    float m_fireballCharge = 0.f;

    static constexpr float kFireballChargeSeconds = 1.0f;
};

} // namespace deadaim