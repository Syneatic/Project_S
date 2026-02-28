#include "camera_components.hpp"
#include "gameobject.hpp"

void MainCamera::OnStart()
{
	CameraSystem::OnStart();
};
void MainCamera::OnUpdate() {
	CameraSystem::MoveCamera(_transform);
};
void MainCamera::OnDestroy() {
	CameraSystem::OnExit();
};

void MainCamera::DrawInInspector() {};
void MainCamera::Serialize(Json::Value& /*outComp*/) const {};
void MainCamera::Deserialize(const Json::Value& /*compObj*/) {};