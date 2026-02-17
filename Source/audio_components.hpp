#pragma once

#include "audio.hpp"
#include "base_components.hpp"

//requires transform
struct AudioEmitter : Behaviour
{
private:
	Audio::Sound _clip;
};

struct AudioListener : Behaviour
{

};