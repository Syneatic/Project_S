#include "gameobject.hpp"
#include "camera.hpp"
#include "scene_manager.hpp"
#include "eventhandler.hpp"
#include "level_transition.hpp"
#include "ui_components.hpp"
#include "render_components.hpp"

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
        SubscribeButton(FunctionKey::PLAY_GAME, [](const UIButtonEvent&) { LevelTransition::RequestTransition(7.f);/*SceneManager::RequestSceneSwitch("TestScene");*/ });
        SubscribeButton(FunctionKey::PAUSE_GAME, [](const UIButtonEvent&) { std::cout << "Pause\n"; });
        SubscribeButton(FunctionKey::RESTART_GAME, [](const UIButtonEvent&) { SceneManager::RequestSceneReload(); });
        SubscribeButton(FunctionKey::QUIT_GAME, [](const UIButtonEvent&) { SceneManager::RequestSceneSwitch("MainMenu"); });
        SubscribeButton(FunctionKey::EXIT_APP, [](const UIButtonEvent&) { EngineCTX::applicationRunning = false; });
    }

    static bool checkBounds(Transform const& t)
    {
        s32 mX{}, mY{}; AEInputGetCursorPosition(&mX, &mY);
        float2 mouseWorld = CameraSystem::ScreenToWorld(float2(mX, mY));
        float2 buttonBounds{ t.position.x - t.scale.x / 2.f, t.position.y - t.scale.y / 2.f };
        bool checkX{ mouseWorld.x > buttonBounds.x && mouseWorld.x < buttonBounds.x + t.scale.x };
        bool checkY{ mouseWorld.y > buttonBounds.y && mouseWorld.y < buttonBounds.y + t.scale.y };
        return checkX && checkY;
    }

    void Hover_Logic(GameObject& button)
    {
        Transform& t = button.transform();
        SpriteRenderer* r = button.GetComponent<SpriteRenderer>();
        if (checkBounds(t))
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
                Button* b = button.GetComponent<Button>();
                EventHandler::RaiseEvent<UIButtonEvent>(b->fKey);
            }
        }
        // set button rgba to default.
        else
            r->color.r = r->color.g = r->color.b = r->color.a;
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