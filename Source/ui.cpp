#include "enginectx.hpp"
#include "gameobject.hpp"
#include "camera.hpp"
#include "scene.hpp"
#include "scene_manager.hpp"
#include "eventhandler.hpp"
#include "level_transition.hpp"
#include "ui_components.hpp"
#include "render_components.hpp"

// Helper functions anon namespace
namespace
{
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

    // 0 is mainMenuHolder, 1 is settingsHolder, 2 is creditsHolder
    std::vector<GameObject*> mainMenuHolders;
    void ToggleSettingsMM()
    {
        mainMenuHolders[0]->active(!mainMenuHolders[0]->active());
        mainMenuHolders[1]->active(!mainMenuHolders[1]->active());
    }

    void ToggleCreditsMM()
    {
        mainMenuHolders[0]->active(!mainMenuHolders[0]->active());
        mainMenuHolders[2]->active(!mainMenuHolders[2]->active());
    }

    // 0 is pauseOverlay, 1 is pauseMenu, 2 is pauseMenuSettings, 3 is endScreenHolder.
    std::vector<GameObject*> pauseMenuHolders;

    void TogglePauseSettingsGame()
    {

    }

    void RestartGame()
    {
        pauseMenuHolders[1]->active(false);
        pauseMenuHolders[3]->active(false);

        EngineCTX::PauseTime();
        LevelTransition::restartCalled = true;
        LevelTransition::RequestTransition();
    }
}

namespace UISystem
{
    std::vector<EventHandler::SubscriptionHandle> handlers;

    // Wrapper for function.
    void SubscribeButton(FunctionKey matchValue, std::function<void(const UIButtonEvent&)> func)
    {
        handlers.push_back(EventHandler::SubscribeFilter(&UIButtonEvent::fKey, matchValue, func));
    }

    // Subscribe each button function as an event to event handler.
    void init()
    {
        if (SceneManager::ActiveScene()->name() == "MainMenu")
        {
            mainMenuHolders.push_back(SceneManager::ActiveScene()->FindGameObjectByName("MainMenuHolder")); //0
            mainMenuHolders.push_back(SceneManager::ActiveScene()->FindGameObjectByName("SettingsHolder")); //1
            mainMenuHolders.back()->active(false);
            mainMenuHolders.push_back(SceneManager::ActiveScene()->FindGameObjectByName("CreditsHolder")); //2
            mainMenuHolders.back()->active(false);

            SubscribeButton(FunctionKey::PLAY_GAME, [](const UIButtonEvent&) { LevelTransition::RequestTransition(); });
            SubscribeButton(FunctionKey::SETTINGS_MM, [](const UIButtonEvent&) { ToggleSettingsMM(); });
            SubscribeButton(FunctionKey::CREDITS_TOGGLE, [](const UIButtonEvent&) { ToggleCreditsMM(); });
            SubscribeButton(FunctionKey::EXIT_APP, [](const UIButtonEvent&) { EngineCTX::applicationRunning = false; });
        }

        if (SceneManager::ActiveScene()->name() == "TestScene")//Change to PlayLevel before checkin.
        {
            pauseMenuHolders.push_back(SceneManager::ActiveScene()->FindGameObjectByName("PauseOverlay")); //0
            pauseMenuHolders.push_back(SceneManager::ActiveScene()->FindGameObjectByName("PauseMenuHolder")); //1
            pauseMenuHolders.push_back(SceneManager::ActiveScene()->FindGameObjectByName("PauseSettingsHolder")); //2
            pauseMenuHolders.push_back(SceneManager::ActiveScene()->FindGameObjectByName("EndScreenHolder")); //3
            for (auto& go : pauseMenuHolders)
            {
                go->active(false);
            }

            SubscribeButton(FunctionKey::PAUSE_GAME, [](const UIButtonEvent&) { TogglePauseMenuGame(); });
            SubscribeButton(FunctionKey::RESTART_GAME, [](const UIButtonEvent&) { RestartGame(); });
            SubscribeButton(FunctionKey::SETTINGS_GAME, [](const UIButtonEvent&) { TogglePauseSettingsGame(); });
            SubscribeButton(FunctionKey::QUIT_GAME, [](const UIButtonEvent&) { LevelTransition::RequestTransition(); });
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

        Transform& t = slider.transform();
        SpriteRenderer* r = slider.gameObject().GetComponent<SpriteRenderer>();
        if (::checkBounds(t))
        {
            r->color.r = r->color.g = r->color.b = r->color.a * .75f;

            // set slider click rgba.
            if (AEInputCheckCurr(AEVK_LBUTTON))
                r->color.r = r->color.g = r->color.b = r->color.a * .5f;
        }
            
        if (AEInputCheckTriggered(AEVK_LBUTTON) && !slider.isDragging)
        {
            if (::checkBounds(t))
                slider.isDragging = true;
        }

        if (AEInputCheckReleased(AEVK_LBUTTON) && slider.isDragging)
        {
            slider.isDragging = false;
            r->color.r = r->color.g = r->color.b = r->color.a;
        }

        if (slider.isDragging)
        {
            float clampedX = std::clamp(::mouseWorld().x, slider.minX, slider.maxX);
            t.position.x = clampedX;

            // Normalize to 0..1
            slider.value = (clampedX - slider.minX) / (slider.maxX - slider.minX);

            // Apply to SFML audio
            //EventHandler::RaiseEvent<UISliderEvent>();
        }
    }

    void TogglePauseMenuGame()
    {
        EngineCTX::PauseTime();
        pauseMenuHolders[1]->active(!pauseMenuHolders[1]->active());
        pauseMenuHolders[0]->active(!pauseMenuHolders[0]->active());
    }

    void EndScreen()
    {
        EngineCTX::PauseTime();
        pauseMenuHolders[0]->active(!pauseMenuHolders[0]->active());
        pauseMenuHolders[3]->active(!pauseMenuHolders[3]->active());
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