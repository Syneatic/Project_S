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
    static void Play() {}
    static void Pause() {}
    static void Restart() {}
    static void Exit() {}

    void BindButtonFunctions(UIButtonRegister& bReg)
    {
        bReg.bindFunction(FunctionKey::PLAY_GAME, Play);
        bReg.bindFunction(FunctionKey::PAUSE_GAME, Pause);
        bReg.bindFunction(FunctionKey::RESTART_GAME, Restart);
        bReg.bindFunction(FunctionKey::EXIT_APP, Exit);
    }

    static bool checkBounds(Transform const& t/*, Alignment const& aignment*/)
    {
        s32 mX{}, mY{}; AEInputGetCursorPosition(&mX, &mY); std::cout << mX << ' ' << mY << std::endl;
        f32 left{ t.position.x - t.scale.x / 2.f }; std::cout << t.position.x << ' ' << t.position.y << std::endl;
        f32 top{ t.position.y - t.scale.y / 2.f }; std::cout << left << ' ' << top << std::endl;
        bool checkX{ static_cast<f32>(mX) > left && static_cast<f32>(mX) < left + t.scale.x };
        bool checkY{ static_cast<f32>(mY) > top && static_cast<f32>(mY) < top + t.scale.y };
        //std::cout << left << ' ' << top << std::endl;
        return checkX && checkY;
    }

    void Hover_Logic(GameObject& button, UIButtonRegister& bReg)
    {
        Transform* t = button.GetComponent<Transform>();
        SpriteRenderer* r = button.GetComponent<SpriteRenderer>();
        if (checkBounds(*t/*, r->alignment*/))
        {
            
            // set button hover rgba.
            if (AEInputCheckTriggered(AEVK_LBUTTON))
            {
                // set button pressed rgba.
            }

            if (AEInputCheckReleased(AEVK_LBUTTON))
            {
                // play button press sfx
                std::cout << "Button Works" << std::endl;
                // Get Button component & use enum key to call function pointer from button key registry.
                /*Button* b = button.GetComponent<Button>();
                bReg.handleMouseClick(b->fKey);*/
            }
        }
        else;// set button default rgba.
    }
}