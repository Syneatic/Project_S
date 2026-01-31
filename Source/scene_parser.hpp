#pragma once
// parse a scene object into a scene file
// read it back from the scene file into object on load

struct Scene;

namespace SceneIO
{
	// ===== Component Serialization =====
    inline bool SerializeComponent(const Component& c, Json::Value& outComp);

    inline void DeserializeComponent(GameObject& go, const Json::Value& compObj);

    // ===== GameObject Serialization =====
    inline Json::Value SerializeGameObject(const GameObject& go);
    
    inline std::unique_ptr<GameObject> DeserializeGameObject(const Json::Value& obj);

    // ===== Scene Serialization =====
    bool SerializeScene(const Scene& scene);

    bool DeserializeScene(Scene& outScene, const std::string& fileNameNoExt);
}