#include <string>
#include <iostream>
#include "ImGUI/imgui.h"
#include "json_parser_helper.hpp"

#include "camera_components.hpp"
#include "gameobject.hpp"


void Camera::DrawInInspector() {}
void Camera::Serialize(Json::Value& outComp) const {}
void Camera::Deserialize(const Json::Value& compObj) {}
void Camera::SetPos(){}

void MainCamera::SetPos(){
	GameObject& owner = *_owner;
	CameraSystem::MoveCamera(*owner.GetComponent<Transform>());
}
