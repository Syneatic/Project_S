/*
Author: Yan Chun
Co-Author: Nil
*/
#include "base_components.hpp"
#include "gameobject.hpp"

Component::Component(GameObject& owner) : 
	_owner(owner), _transform(owner.transform()), _worldTransform(owner.worldTransform()) {}