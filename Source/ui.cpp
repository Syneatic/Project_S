#include "ui_types.hpp"
#include "render_components.hpp"
#include "color.hpp"
#include "gameobject.hpp"

#include "AEEngine.h"
#include "AEInput.h"
#include "AETypes.h"

namespace UISystem
{
    // Functions for UI buttons.
    static void Play() 
    {
        //SceneManager& instance{ SceneManager::Instance() };
        std::cout << "Play Game\n";
    }
    static void Pause() {}
    static void Restart() {}
    static void Exit() {}

    void BindButtonFunctions(UIButtonRegister& bReg)
    {
        std::cout << "Try Bind\n";
        bReg.bindFunction(FunctionKey::PLAY_GAME, Play);
        bReg.bindFunction(FunctionKey::PAUSE_GAME, Pause);
        bReg.bindFunction(FunctionKey::RESTART_GAME, Restart);
        bReg.bindFunction(FunctionKey::EXIT_APP, Exit);
    }

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

    static bool checkBounds(Transform const& t/*, Alignment const& aignment*/)
    {
        s32 mX{}, mY{}; AEInputGetCursorPosition(&mX, &mY);
        float2 mouseWorld{ScreenToWorld(mX, mY)};
        float2 buttonBounds{ t.position.x - t.scale.x / 2.f, t.position.y - t.scale.y / 2.f };
        bool checkX{ mouseWorld.x > buttonBounds.x && mouseWorld.x < buttonBounds.x + t.scale.x };
        bool checkY{ mouseWorld.y > buttonBounds.y && mouseWorld.y < buttonBounds.y + t.scale.y };
        return checkX && checkY;
    }

    void Hover_Logic(GameObject& button, UIButtonRegister& bReg)
    {
        Transform* t = button.GetComponent<Transform>();
        SpriteRenderer* r = button.GetComponent<SpriteRenderer>();
        Color base{ r->color };
        if (checkBounds(*t/*, r->alignment*/))
        {
            r->color.r = r->color.g = r->color.b = base.a * .75f;
            
            if (AEInputCheckTriggered(AEVK_LBUTTON))
            {
                // play button press sfx
            }

            // set button hover rgba.
            if (AEInputCheckCurr(AEVK_LBUTTON))
            {
                r->color.r = r->color.g = r->color.b = base.a * .5f;
            }

            if (AEInputCheckReleased(AEVK_LBUTTON))
            {
                // Get Button component & use enum key to call function pointer from button key registry.
                Button* b = button.GetComponent<Button>();
                bReg.handleMouseClick(b->fKey);
            }                       
        }      
        else
            r->color = base;
    }
}