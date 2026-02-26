#pragma once
// parse a scene object into a scene file
// read it back from the scene file into object on load

struct Scene;
struct GameObject;
struct Component;

namespace SceneIO
{
	// ===== Component Serialization =====
    bool SerializeComponent(const Component& c, Json::Value& outComp);

    void DeserializeComponent(GameObject& go, const Json::Value& compObj);

    // ===== GameObject Serialization =====
    Json::Value SerializeGameObject(const GameObject& go);
    
    std::unique_ptr<GameObject> DeserializeGameObject(const Json::Value& obj);

    // ===== Scene Serialization =====
    bool SerializeScene(const Scene& scene);

    bool DeserializeScene(Scene& outScene, const std::string& fileNameNoExt);

    bool DeserializeSceneEditor(Scene& outScene, const std::string& fileNameNoExt);

}