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
    GameOver,
    SettingsMenu,
    Paused

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
    void applySettings(const Settings& settings);
    Window m_window;
    Renderer m_renderer;
    AssetManager m_assetManager; 
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
    void updateMenus(sf::Vector2f mouseDesignPosition);
    void resumeFromPause();
    struct PlayerIntent {
        sf::Vector2f aimTarget;
        bool shootRequest = false;
        bool visionTracking = false;
        GestureType gesture = GestureType::None;
        float fireballCharge = 0.f;
      
        
    };

    PlayerIntent resolveInput(float dt, sf::Vector2f mouseDesignPosition);
    int m_lastHealth = 100;
    int m_lastScore = 0;
    bool m_holdWasActive = false;  
    bool m_visionWasTracking = false;
    float m_fireballCharge = 0.f;

    static constexpr float kFireballChargeSeconds = 1.0f;
};

} 