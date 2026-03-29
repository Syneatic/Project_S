#include "enginectx.hpp"
#include "gameobject.hpp"
#include "camera.hpp"
#include "scene.hpp"
#include "audio.hpp"
#include "save_game.hpp"
#include "scene_manager.hpp"
#include "eventhandler.hpp"
#include "level_transition.hpp"
#include "ui_components.hpp"
#include "render_components.hpp"
#include <string>

// Helper anon namespace for game object holders.
namespace
{
    struct
    {
        GameObject* mainMenu = nullptr, * mainSettings = nullptr,
            * mainCredits = nullptr, * mainControls = nullptr,
            * mouseParticle = nullptr;
    }mainMenuHolders;

    struct
    {
        GameObject* pauseOverlay = nullptr, * pauseMenu = nullptr,
            * pauseSettings = nullptr, * pauseControls = nullptr,
            * gameTimer = nullptr, * endScreen = nullptr;
    }pauseMenuHolders;

    TextRenderer* endScreenHeader = nullptr;

    void SetEndScreenText(bool state)
    {
        endScreenHeader->text = state ? "YOU WIN" : "YOU LOSE";
    }

    void ToggleUI(GameObject* go) { go->active(!go->active()); }
    void ToggleUI(bool state, GameObject* go) { go->active(state); }

    void ToggleUIPair(GameObject* one, GameObject* two)
    {
        if (one && two)
        {
            one->active(!one->active());
            two->active(!two->active());
        }
    }
    void ToggleUIPair(bool state, GameObject* one, GameObject* two)
    {
        if (one && two)
        {
            one->active(state);
            two->active(state);
        }
    }
}

// Helper functions anon namespace
namespace
{
    std::vector<EventHandler::SubscriptionHandle> handlers;

    // Wrapper for function.
    void SubscribeButton(FunctionKey matchValue, std::function<void(const UIButtonEvent&)> func)
    {
        handlers.push_back(EventHandler::SubscribeFilter(&UIButtonEvent::fKey, matchValue, func));
    }

    void SubscribeSlider(AudioSpecifier matchValue, std::function<void(const UISliderEvent&)> func)
    {
        handlers.push_back(EventHandler::SubscribeFilter(&UISliderEvent::aS, matchValue, func));
    }

    float2 mouseWorld()
    {
        s32 mX{}, mY{}; AEInputGetCursorPosition(&mX, &mY);
        return { mX - AEGfxGetWindowWidth() / 2.f, -(mY - AEGfxGetWindowHeight() / 2.f) };
    }

    bool checkBounds(Transform const& t)
    {
        float2 const& mouseWorld = ::mouseWorld();
        float2 buttonBounds{ t.position.x - t.scale.x / 2.f, t.position.y - t.scale.y / 2.f };
        bool checkX{ mouseWorld.x > buttonBounds.x && mouseWorld.x < buttonBounds.x + t.scale.x };
        bool checkY{ mouseWorld.y > buttonBounds.y && mouseWorld.y < buttonBounds.y + t.scale.y };
        return checkX && checkY;
    }

    void RestartGame()
    {
        LevelTransition::restartCalled = true;
        LevelTransition::RequestTransition();
    }

    void UIMainMenu()
    {
        mainMenuHolders.mainMenu = SceneManager::ActiveScene()->FindGameObjectByName("MainMenuHolder");
        mainMenuHolders.mainSettings = SceneManager::ActiveScene()->FindGameObjectByName("SettingsHolder");
        mainMenuHolders.mainCredits = SceneManager::ActiveScene()->FindGameObjectByName("CreditsHolder");
        mainMenuHolders.mainControls = SceneManager::ActiveScene()->FindGameObjectByName("ControlsHolder");
        mainMenuHolders.mouseParticle = SceneManager::ActiveScene()->FindGameObjectByName("MouseParticle");
        mainMenuHolders.mainSettings->active(false); mainMenuHolders.mainCredits->active(false);
        mainMenuHolders.mainControls->active(false);

        SubscribeButton(FunctionKey::PLAY_GAME, [](const UIButtonEvent&)
            { LevelTransition::RequestTransition(); SaveGameManager::toLoad = false; });
        SubscribeButton(FunctionKey::LOAD_GAME, [](const UIButtonEvent&)
            { 
                if (std::ifstream{ "Saves/Play_Level.save", std::ios::binary })
                { LevelTransition::RequestTransition(); SaveGameManager::toLoad = true; }               
            });
        SubscribeButton(FunctionKey::SETTINGS_MM, [](const UIButtonEvent&)
            { ToggleUIPair(mainMenuHolders.mainMenu, mainMenuHolders.mainSettings); });
        SubscribeButton(FunctionKey::CREDITS_TOGGLE, [](const UIButtonEvent&)
            { ToggleUIPair(mainMenuHolders.mainMenu, mainMenuHolders.mainCredits); });
        SubscribeButton(FunctionKey::CONTROLS_MM, [](const UIButtonEvent&)
            { ToggleUIPair(mainMenuHolders.mainMenu, mainMenuHolders.mainControls); });
        SubscribeButton(FunctionKey::EXIT_APP, [](const UIButtonEvent&)
            { EngineCTX::applicationRunning = false; });
    }

    void UIPlayLevel()
    {
        pauseMenuHolders.pauseOverlay = SceneManager::ActiveScene()->FindGameObjectByName("PauseOverlay");
        pauseMenuHolders.pauseMenu = SceneManager::ActiveScene()->FindGameObjectByName("PauseMenuHolder");
        pauseMenuHolders.pauseSettings = SceneManager::ActiveScene()->FindGameObjectByName("PauseSettingsHolder");
        pauseMenuHolders.pauseControls = SceneManager::ActiveScene()->FindGameObjectByName("PauseControlsHolder");
        pauseMenuHolders.gameTimer = SceneManager::ActiveScene()->FindGameObjectByName("TimerValue");
        pauseMenuHolders.endScreen = SceneManager::ActiveScene()->FindGameObjectByName("EndScreenHolder");
        endScreenHeader = SceneManager::ActiveScene()->FindGameObjectByName("EndScreenHeader")->GetComponent<TextRenderer>();
        ToggleUIPair(false, pauseMenuHolders.pauseOverlay, pauseMenuHolders.pauseMenu);
        ToggleUIPair(false, pauseMenuHolders.pauseSettings, pauseMenuHolders.endScreen);

        SubscribeButton(FunctionKey::PAUSE_GAME, [](const UIButtonEvent&)
            { UISystem::TogglePauseMenuGame(); });
        SubscribeButton(FunctionKey::RESTART_GAME, [](const UIButtonEvent&)
            { RestartGame(); SaveGameManager::toLoad = true; });
        SubscribeButton(FunctionKey::SETTINGS_GAME, [](const UIButtonEvent&)
            { if (EngineCTX::isPaused) ToggleUIPair(pauseMenuHolders.pauseMenu, pauseMenuHolders.pauseSettings); });
        SubscribeButton(FunctionKey::CONTROLS_GAME, [](const UIButtonEvent&)
            { if (EngineCTX::isPaused) ToggleUIPair(pauseMenuHolders.pauseMenu, pauseMenuHolders.pauseControls); });
        SubscribeButton(FunctionKey::QUIT_GAME, [](const UIButtonEvent&)
            { LevelTransition::RequestTransition(); });
    }
}

namespace UISystem
{
    // Subscribe each button function as an event to event handler.
    void init()
    {
        if (SceneManager::ActiveScene()->name() == "MainMenu")
            UIMainMenu();

        if (SceneManager::ActiveScene()->name() == "Play_Level")//Change to PlayLevel before checkin.
            UIPlayLevel();

        if (SceneManager::ActiveScene()->name() != "Intro")
        {
            SubscribeSlider(AudioSpecifier::GLOBAL, [](UISliderEvent const& e)
                { Audio::SetMasterVolume(e.value); });
            SubscribeSlider(AudioSpecifier::SFX, [](UISliderEvent const& e)
                { Audio::SetSFXVolume(e.value); });
            SubscribeSlider(AudioSpecifier::MUSIC, [](UISliderEvent const& e)
                { Audio::SetMusicVolume(e.value); });
        }
    }

    void Update()
    {
        if (SceneManager::ActiveScene()->name() == "MainMenu") 
            mainMenuHolders.mouseParticle->transform().position = mouseWorld();

        if (SceneManager::ActiveScene()->name() == "Play_Level")
        {
            EngineCTX::gameTimer += EngineCTX::dt;
            pauseMenuHolders.gameTimer->GetComponent<TextRenderer>()->text 
                = std::to_string(static_cast<float>(EngineCTX::gameTimer)) + "Sec";
        }                  
    }

    // Logic for button click.
    void Hover_Logic(Button& button)
    {
        if (LevelTransition::inTransition)
            return;

        Transform const& t = button.transform();
        SpriteRenderer* r = button.gameObject().GetComponent<SpriteRenderer>();
        if (::checkBounds(t))
        {
            // set button hover rgba.
            r->color.r = r->color.g = r->color.b = r->color.a * .75f;

            if (AEInputCheckTriggered(AEVK_LBUTTON))
            {
                // play button press sfx
            }

            // set button click rgba.
            if (AEInputCheckCurr(AEVK_LBUTTON))
            {
                r->color.r = r->color.g = r->color.b = r->color.a * .5f;
            }

            if (AEInputCheckReleased(AEVK_LBUTTON))
            {
                // Get Button component & use enum key to raise UIButtonEvent on EventHandler. 
                EventHandler::RaiseEvent<UIButtonEvent>(button.fKey);
                r->color.r = r->color.g = r->color.b = r->color.a;
            }
        }
        // set button rgba to default.
        else
            r->color.r = r->color.g = r->color.b = r->color.a;
    }

    // Overload for slider.
    void Hover_Logic(Slider& slider)
    {
        if (LevelTransition::inTransition)
            return;

        Transform const& wtSlider = slider.worldTransform();
        Transform& tSlider = slider.transform();
        SpriteRenderer* r = slider.gameObject().GetComponent<SpriteRenderer>();

        if (::checkBounds(wtSlider))
        {
            r->color.r = r->color.g = r->color.b = r->color.a * .75f;

            // set slider click rgba.
            if (AEInputCheckCurr(AEVK_LBUTTON))
                r->color.r = r->color.g = r->color.b = r->color.a * .5f;
        }
            
        if (AEInputCheckTriggered(AEVK_LBUTTON) && !slider.isDragging)
        {
            if (::checkBounds(wtSlider))
                slider.isDragging = true;
        }

        if (AEInputCheckReleased(AEVK_LBUTTON) && slider.isDragging)
        {
            slider.isDragging = false;
            r->color.r = r->color.g = r->color.b = r->color.a;
        }

        if (slider.isDragging)
        {
            f32 clampedX = std::clamp(::mouseWorld().x, slider.minX, slider.maxX);
            tSlider.position.x = clampedX / slider.TrackTransform().scale.x;

            // Normalize to 0..1
            slider.value = (clampedX - slider.minX) / (slider.maxX - slider.minX);

            // Apply to SFML audio
            EventHandler::RaiseEvent<UISliderEvent>(slider.audioS, slider.value);
        }
    }

    void TogglePauseMenuGame()
    {
        if (LevelTransition::inTransition)
            return;

        EngineCTX::PauseTime();
        ToggleUIPair(pauseMenuHolders.pauseMenu, pauseMenuHolders.pauseOverlay);

        if (pauseMenuHolders.pauseSettings->active())
            ToggleUIPair(false, pauseMenuHolders.pauseMenu, pauseMenuHolders.pauseSettings);

        if (pauseMenuHolders.pauseControls->active())
            ToggleUIPair(false, pauseMenuHolders.pauseMenu, pauseMenuHolders.pauseControls);
    }

    void EndScreen(bool state)
    {
        SetEndScreenText(state);
        EngineCTX::PauseTime();
        ToggleUIPair(pauseMenuHolders.endScreen, pauseMenuHolders.pauseOverlay);
    }

    GameObject& GetTimer()
    {
        return *pauseMenuHolders.gameTimer;
    }

    // test function for unsub.
    void exit()
    {
        for (auto& eh : handlers)
        {
            EventHandler::Unsubscribe(eh);
        }
        handlers.clear();
    }
}