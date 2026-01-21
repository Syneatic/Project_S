#include "ui_types.hpp"
#include "color.hpp"
#include "gameobject.hpp"

#include "AEEngine.h"
#include "AEInput.h"
#include "AETypes.h"

void Hover_Logic(GameObject &button)
{
    Transform* t = button.GetComponent<Transform>(); // Later got time see if can move get component elsewhere.
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
            Button* b = button.GetComponent<Button>();
            b->callFunc;
        }
    }
    else;// set button default rgba.
}