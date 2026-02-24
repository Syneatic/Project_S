#pragma once
#include "AEEngine.h"

#include "camera.hpp"
#include "base_components.hpp"


//abstract
struct Camera : Component
{
    virtual void SetPos();
};