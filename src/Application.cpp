#include "Application.h"
#include "VisionClient.h"
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <algorithm>
#include <optional>

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

    MenuInfo info;
    info.highScore = m_saveSystem.getHighScore();
    m_menu.setMode(MenuScreen::Mode::MainMenu, info);

    m_visionClient.connect("127.0.0.1", 50505);
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
    // Updated in every state: the fog should breathe behind the menus
    // too, not just during play.
    m_environment.update(dt);

    sf::Vector2f mouseDesignPosition =
        m_renderer.mapPixelToDesignSpace(m_window.getMousePixelPosition());
    // ... rest unchanged    
    switch (m_state) {
    case GameState::MainMenu:
    case GameState::GameOver: {
        bool clicked = m_window.consumeLeftClick();
MenuScreen::Action action = m_menu.update(mouseDesignPosition, clicked);

if (action != MenuScreen::Action::None) {
    m_audio.play(SoundId::MenuClick);
}

if (action == MenuScreen::Action::StartGame) {
    resetGame();
    m_audio.startMusic();
    m_state = GameState::Playing;
} else if (action == MenuScreen::Action::Quit) {
    m_window.close();
}

break;
    }

    case GameState::Playing:
        updatePlaying(dt, mouseDesignPosition);
        break;
    }
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

        // Sticky weapon selection: a recognised gesture switches the
        // weapon and it STAYS switched. If a weapon were equipped only
        // while its gesture was held, releasing to fire would un-equip
        // it in the same instant -- the shot would race the switch.
        switch (intent.gesture) {
        case GestureType::Gun:      m_scene.getPlayer().equipWeapon(WeaponType::Gun); break;
        case GestureType::Bow:      m_scene.getPlayer().equipWeapon(WeaponType::Bow); break;
        case GestureType::Fireball: m_scene.getPlayer().equipWeapon(WeaponType::Fireball); break;
        case GestureType::None:     break; // keep whatever is already equipped
        }
    }

    // Hand vanished mid-hold: cancel rather than loosing a shot at
    // wherever the crosshair happened to freeze. Checked before the
    // per-weapon logic so the falling edge below can't trigger.
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
        // Fires on the RELEASE -- the tick where the pinch was held and
        // now isn't. Detected here rather than sent as an event from
        // Python: state is idempotent, so re-reading the same message
        // can't fire twice.
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
            m_fireballCharge = 0.f; // released early: fizzles, starts over
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
    m_hud.update(hudState);
    m_weaponView.update(dt, m_scene.getPlayer().getEquippedWeapon(),
                        m_scene.consumeShotFired());
if (!m_scene.getPlayer().isAlive()) {
        int finalScore = m_scene.getScore();

        MenuInfo info;
        info.finalScore = finalScore;
        info.isNewHighScore = m_saveSystem.submitScore(finalScore);
        info.highScore = m_saveSystem.getHighScore();

        // Both plays live inside this block because the block is already
        // once-only: it ends by setting m_state to GameOver, and
        // updatePlaying() only runs while the state is Playing. No flag,
        // timer, or extra state is needed to stop a repeat.
        m_audio.play(SoundId::GameOver);

        // submitScore() returns true only when this beat the stored
        // record, so it is exactly the "new best" condition. Reusing the
        // same value the menu reads means the sound can never disagree
        // with the gold NEW HIGH SCORE! text about to appear.
        if (info.isNewHighScore) {
            m_audio.play(SoundId::NewHighScore);
        }

        m_menu.setMode(MenuScreen::Mode::GameOver, info);
        m_state = GameState::GameOver;
        m_holdWasActive = false;   // don't carry a half-drawn bow or
        m_fireballCharge = 0.f;    // half-charged fireball into the next run
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

    // Submitted first, but that's incidental -- Renderer buckets by layer,
    // so the backdrop would land behind the world and the fog in front of
    // it no matter what order these calls happen in.
    m_environment.render(m_renderer);

    // Game Over keeps the frozen scene visible behind the overlay -- the
    // player gets to see what killed them.
    if (m_state == GameState::Playing || m_state == GameState::GameOver) {
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

} // namespace deadaim