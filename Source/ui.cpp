#include "ui_types.hpp"
#include "color.hpp"
#include "gameobject.hpp"

#include "AEEngine.h"
#include "AEInput.h"
#include "AETypes.h"

// Preset value.
constexpr f32 defaultButtonHeight{ 100.f }, defaultButtonWidth{ 400.f }, defaultTextSize{ 40.f }, defaultStrokeWeight{ 2.f }, zeroVal{};

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

void Hover_Logic(GameObject &button, UIButtonRegister& bReg)
{
    Transform* t = button.GetComponent<Transform>();
    s32 mX{}, mY{}; AEInputGetCursorPosition(&mX, &mY);
    bool xCheck{};// Implement selection for center & lside.
    bool YCheck{};// Implement selection for center & lside.
    if (xCheck && YCheck)
    {
        // set button hover rgba.
        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            // set button pressed rgba.
        }

        if (AEInputCheckReleased(AEVK_LBUTTON))
        {
            // play button press sfx
            // 
            // Get Button component & use enum key to call function pointer from button key registry.
            Button* b = button.GetComponent<Button>();
            bReg.handleMouseClick(b->fKey);
        }
    }
    else;// set button default rgba.
}