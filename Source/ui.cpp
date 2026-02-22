#include <iostream>

#include "AEEngine.h"

#include "gameobject.hpp"
#include "eventhandler.hpp"
#include "ui_components.hpp"
#include "render_components.hpp"

namespace UISystem
{
    //void BindButtonFunctions(UIButtonRegister& bReg)
    //{
    //    bReg.bindFunction(FunctionKey::PLAY_GAME, Play);
    //    bReg.bindFunction(FunctionKey::PAUSE_GAME, Pause);
    //    bReg.bindFunction(FunctionKey::RESTART_GAME, Restart);
    //    bReg.bindFunction(FunctionKey::QUIT_GAME, Quit);
    //    bReg.bindFunction(FunctionKey::EXIT_APP, Exit);
    //}

    float2 ScreenToWorld(s32 x, s32 y)
    {
        float2 screenToWorld
        {
            AEGfxGetWinMinX() + static_cast<f32>(x),
            AEGfxGetWinMaxY() - static_cast<f32>(y)
        };

        // Add camera translate in future.

        return screenToWorld;
    }

    static bool checkBounds(Transform const& t)
    {
        s32 mX{}, mY{}; AEInputGetCursorPosition(&mX, &mY);
        float2 mouseWorld{ScreenToWorld(mX, mY)};
        float2 buttonBounds{ t.position.x - t.scale.x / 2.f, t.position.y - t.scale.y / 2.f };
        bool checkX{ mouseWorld.x > buttonBounds.x && mouseWorld.x < buttonBounds.x + t.scale.x };
        bool checkY{ mouseWorld.y > buttonBounds.y && mouseWorld.y < buttonBounds.y + t.scale.y };
        return checkX && checkY;
    }

    void Hover_Logic(GameObject& button)
    {
        Transform* t = button.GetComponent<Transform>();
        SpriteRenderer* r = button.GetComponent<SpriteRenderer>();
        if (checkBounds(*t))
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
}