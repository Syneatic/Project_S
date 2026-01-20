#include "ui_types.hpp"
#include "gameobject.hpp"
#include "AEEngine.h"
#include "AEInput.h"
#include "AETypes.h"

void Hover_Logic(GameObject &button, s32 mX, s32 mY)
{
    if (/* logic to get object transform pos*/true)
    {
        // set button hover rgba.
        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            // set button pressed rgba.
        }

        if (AEInputCheckReleased(AEVK_LBUTTON))
        {
            // play button press sfx
            // get button component & call function pointer
        }
    }
    else;// set button default rgba.
}