#include "Application.h"
#include "VisionClient.h"
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <algorithm>
#include <optional>
#include "Settings.h"

namespace deadaim {

namespace {
constexpr float kDesignWidth = 1920.f;
constexpr float kDesignHeight = 1080.f;
}

Application::Application()
    : m_window(1920, 1080, "DeadAim")
    , m_renderer(m_window.getNativeWindow())
    , m_waveSystem(m_assetManager)
    , m_hud(m_assetManager)
    , m_menu(m_assetManager)
    , m_weaponView(m_assetManager)
{
    m_saveSystem.load();
    m_menu.setSettings(m_saveSystem.getSettings());
    applySettings(m_saveSystem.getSettings());
    MenuInfo info;
    info.highScore = m_saveSystem.getHighScore();
    m_menu.setMode(MenuScreen::Mode::MainMenu, info);

    m_visionClient.connect("127.0.0.1", 50505);
}
void Application::applySettings(const Settings& settings) {
    m_audio.setMusicVolume(settings.musicVolume);
    m_audio.setSfxVolume(settings.sfxVolume);
    m_scene.getPlayer().setSensitivity(settings.sensitivity);
}
void Application::run() {
    sf::Clock clock;
    float accumulator = 0.0f;

    while (m_window.isOpen()) {
        m_window.processEvents();

        float frameTime = clock.restart().asSeconds();
        frameTime = std::min(frameTime, 0.25f);
        accumulator += frameTime;

        while (accumulator >= kFixedTimestep) {
            update(kFixedTimestep);
            accumulator -= kFixedTimestep;
        }

        render();
    }
}


    
void Application::update(float dt) {
    m_environment.update(dt);

    sf::Vector2f mouseDesignPosition =
        m_renderer.mapPixelToDesignSpace(m_window.getMousePixelPosition());

    bool escapePressed = m_window.consumeEscapePressed();

    switch (m_state) {
    case GameState::Playing:
        if (escapePressed) {
            MenuInfo info;
            info.finalScore = m_scene.getScore();
            m_menu.setMode(MenuScreen::Mode::Paused, info);
            m_audio.pauseMusic();
            m_audio.play(SoundId::MenuClick);
            m_state = GameState::Paused;
            m_window.consumeLeftClick();
            break;
        }
        updatePlaying(dt, mouseDesignPosition);
        break;

    case GameState::SettingsMenu:
        if (escapePressed) {
            MenuInfo info;
            info.highScore = m_saveSystem.getHighScore();
            m_menu.setMode(MenuScreen::Mode::MainMenu, info);
            m_audio.play(SoundId::MenuClick);
            m_state = GameState::MainMenu;
            m_window.consumeLeftClick();
            break;
        }
        updateMenus(mouseDesignPosition);
        break;

    case GameState::Paused:
        if (escapePressed) {
            resumeFromPause();
            m_window.consumeLeftClick();
            break;
        }
        updateMenus(mouseDesignPosition);
        break;

    case GameState::MainMenu:
    case GameState::GameOver:
        updateMenus(mouseDesignPosition);
        break;
    }
}

void Application::updateMenus(sf::Vector2f mouseDesignPosition) {
    bool clicked = m_window.consumeLeftClick();
    MenuScreen::Action action = m_menu.update(mouseDesignPosition, clicked);

    if (action != MenuScreen::Action::None && action != MenuScreen::Action::SettingsChanged) {
        m_audio.play(SoundId::MenuClick);
    }

    switch (action) {
    case MenuScreen::Action::StartGame:
        resetGame();
        applySettings(m_saveSystem.getSettings());
        m_audio.resumeMusic();
        m_audio.startMusic();
        m_state = GameState::Playing;
        break;

    case MenuScreen::Action::Resume:
        resumeFromPause();
        break;

    case MenuScreen::Action::ToMainMenu: {
        m_audio.stopMusic();
        MenuInfo info;
        info.highScore = m_saveSystem.getHighScore();
        m_menu.setMode(MenuScreen::Mode::MainMenu, info);
        m_state = GameState::MainMenu;
        break;
    }

    case MenuScreen::Action::OpenSettings:
        m_menu.setMode(MenuScreen::Mode::Settings);
        m_state = GameState::SettingsMenu;
        break;

    case MenuScreen::Action::SettingsChanged:
        m_saveSystem.updateSettings(m_menu.getSettings());
        applySettings(m_menu.getSettings());
        break;

    case MenuScreen::Action::CloseSettings: {
        MenuInfo info;
        info.highScore = m_saveSystem.getHighScore();
        m_menu.setMode(MenuScreen::Mode::MainMenu, info);
        m_state = GameState::MainMenu;
        break;
    }

    case MenuScreen::Action::Quit:
        m_window.close();
        break;

    case MenuScreen::Action::None:
        break;
    }
}

void Application::resumeFromPause() {
    m_holdWasActive = false;
    m_fireballCharge = 0.f;
    m_visionWasTracking = false;
    m_audio.resumeMusic();
    m_state = GameState::Playing;
}

Application::PlayerIntent Application::resolveInput(float dt, sf::Vector2f mouseDesignPosition) {
    PlayerIntent intent;
    intent.aimTarget = mouseDesignPosition;

    // Mouse stays available throughout as a fallback: keeps the game
    // playable and debuggable when the vision process isn't running.
    bool mouseHeld = m_window.isLeftMouseButtonHeld();

    std::optional<VisionData> visionData = m_visionClient.getLatestData();
    intent.visionTracking = visionData && visionData->tracking;
    intent.gesture = intent.visionTracking ? visionData->gesture : GestureType::None;

    if (intent.visionTracking) {
        intent.aimTarget = {visionData->aimX * kDesignWidth, visionData->aimY * kDesignHeight};

        switch (intent.gesture) {
        case GestureType::Gun:      m_scene.getPlayer().equipWeapon(WeaponType::Gun); break;
        case GestureType::Bow:      m_scene.getPlayer().equipWeapon(WeaponType::Bow); break;
        case GestureType::Fireball: m_scene.getPlayer().equipWeapon(WeaponType::Fireball); break;
        case GestureType::None:     break; // keep whatever is already equipped
        }
    }

    if (m_visionWasTracking && !intent.visionTracking) {
        m_holdWasActive = false;
        m_fireballCharge = 0.f;
    }
    m_visionWasTracking = intent.visionTracking;

    switch (m_scene.getPlayer().getEquippedWeapon()) {
    case WeaponType::Gun: {
        m_holdWasActive = false;
        m_fireballCharge = 0.f;
        bool triggerPulled = (intent.gesture == GestureType::Gun)
                             && visionData && visionData->firing;
        intent.shootRequest = triggerPulled || mouseHeld;
        break;
    }

    case WeaponType::Bow: {
        m_fireballCharge = 0.f;
        
        bool holdNow = (intent.gesture == GestureType::Bow) || mouseHeld;
        if (m_holdWasActive && !holdNow) {
            intent.shootRequest = true;
        }
        m_holdWasActive = holdNow;
        break;
    }

    case WeaponType::Fireball: {
        bool holdNow = (intent.gesture == GestureType::Fireball) || mouseHeld;
        if (holdNow) {
            m_fireballCharge = std::min(m_fireballCharge + dt / kFireballChargeSeconds, 1.f);
        }
        if (m_holdWasActive && !holdNow) {
            if (m_fireballCharge >= 1.f) {
                intent.shootRequest = true;
            }
            m_fireballCharge = 0.f; 
        }
        m_holdWasActive = holdNow;
        break;
    }
    }

    intent.fireballCharge = m_fireballCharge;
    return intent;
}

void Application::updatePlaying(float dt, sf::Vector2f mouseDesignPosition) {
    PlayerIntent intent = resolveInput(dt, mouseDesignPosition);

    m_scene.update(dt, intent.aimTarget, intent.shootRequest, m_audio);
    m_collisionSystem.update(m_scene, m_audio);
    m_waveSystem.update(dt, m_scene);
    
    int health = m_scene.getPlayer().getHealth();
    int score = m_scene.getScore();
    bool playerHurt = health < m_lastHealth;
    if (health < m_lastHealth) {
        m_audio.play(SoundId::PlayerHurt);
    }

    if (score > m_lastScore) {
        m_audio.play(SoundId::ZombieDeath);
    }
    m_lastHealth = health;
    m_lastScore = score;
    
    HudState hudState;
    hudState.health = m_scene.getPlayer().getHealth();
    hudState.maxHealth = m_scene.getPlayer().getMaxHealth();
    hudState.score = m_scene.getScore();
    hudState.wave = m_waveSystem.getCurrentWave();
    hudState.weaponName = m_scene.getPlayer().getWeaponName();
    hudState.visionTracking = intent.visionTracking;
    hudState.inGunMode = intent.gesture == GestureType::Gun;
    hudState.bowDrawn = intent.gesture == GestureType::Bow;
    hudState.fireballCharge = intent.fireballCharge;
    hudState.playerJustHurt = playerHurt;
    m_hud.update(dt , hudState);
    m_weaponView.update(dt, m_scene.getPlayer().getEquippedWeapon(),
                        m_scene.consumeShotFired());
if (!m_scene.getPlayer().isAlive()) {
        int finalScore = m_scene.getScore();

        MenuInfo info;
        info.finalScore = finalScore;
        info.isNewHighScore = m_saveSystem.submitScore(finalScore);
        info.highScore = m_saveSystem.getHighScore();

        m_audio.play(SoundId::GameOver);

        if (info.isNewHighScore) {
            m_audio.play(SoundId::NewHighScore);
        }

        m_menu.setMode(MenuScreen::Mode::GameOver, info);
        m_state = GameState::GameOver;
        m_holdWasActive = false;  
        m_fireballCharge = 0.f;    
        m_window.consumeLeftClick();
    }
}

void Application::resetGame() {
    m_scene.reset();
    m_waveSystem.reset();

    m_lastHealth = m_scene.getPlayer().getMaxHealth();
    m_lastScore = 0;
}

void Application::render() {
    m_renderer.beginFrame(sf::Color(20, 20, 25));
    m_renderer.applyLetterbox();
  
    m_environment.render(m_renderer);

    if (m_state == GameState::Playing || m_state == GameState::GameOver
    || m_state == GameState::Paused) {
        m_scene.render(m_renderer);
        m_weaponView.render(m_renderer);
        m_hud.render(m_renderer);
    }
    if (m_state != GameState::Playing) {
        m_menu.render(m_renderer);
    }

    m_renderer.endFrame();
    m_window.display();
}

} 