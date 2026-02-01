#pragma once
// parse a scene object into a scene file
// read it back from the scene file into object on load
#include "json.h"

struct Scene;
struct GameObject;
struct Component;

namespace SceneIO
{
	// ===== Component Serialization =====
    bool SerializeComponent(const Component& c, Json::Value& outComp);

    void DeserializeComponent(GameObject& go, const Json::Value& compObj);
    {
        //skip unknown components
        //if (type != "Transform" && type != "CircleCollider" && type != "BoxCollider")
        //    return false;

        outComp = Value(objectValue);
        outComp["type"] = type;
        
        c.Serialize(outComp);

        return true;
    }

    // inline void DeserializeComponent(GameObject& go, const Value& compObj)
    // {
    //     if (!compObj.isObject()) return;
    //     if (!compObj.isMember("type") || !compObj["type"].isString()) return;

    //     const std::string type = compObj["type"].asString();

    //     if (type == "Transform")
    //     {
    //         Transform t{};
    //         t.Deserialize(compObj);
    //         go.AddComponent<Transform>(t);
    //     }
    //     else if (type == "CircleCollider")
    //     {
    //         CircleCollider c{};
    //         c.Deserialize(compObj);
    //         go.AddComponent<CircleCollider>(c);
    //     }
    //     else if (type == "BoxCollider")
    //     {
    //         BoxCollider b{};
    //         b.Deserialize(compObj);
    //         go.AddComponent<BoxCollider>(b);
    //     }
    //     else if (type == "SpriteRenderer")
    //     {
    //         SpriteRenderer r{};
    //         r.Deserialize(compObj);
    //         go.AddComponent<SpriteRenderer>(r);
    //     }
    //     else if (type == "PlayerController")
    //     {
    //         PlayerController pc{};
    //         pc.Deserialize(compObj);
    //         go.AddComponent<PlayerController>(pc);
    //     }
    //     else if (type == "Text")
    //     {
    //         Text str{};
    //         str.Deserialize(compObj);
    //         go.AddComponent<Text>(str);
    //     }
    //     else if (type == "EchoPingTest")
    //     {
    //         EchoPingTest ept{};
    //         ept.Deserialize(compObj);
    //         go.AddComponent<EchoPingTest>(ept);
    //     }
    //     else if (type == "Button")
    //     {
    //         Button b{};
	// 		b.Deserialize(compObj);
    //         go.AddComponent<Button>(b);
    //     }
    //     if (type == "RigidBody")
    //     {
    //         RigidBody rb{};
    //         rb.Deserialize(compObj);
    //         go.AddComponent<RigidBody>(rb);
    //     }
    // }

    // ===== GameObject Serialization =====
    Json::Value SerializeGameObject(const GameObject& go);
    
    std::unique_ptr<GameObject> DeserializeGameObject(const Json::Value& obj);

    // ===== Scene Serialization =====
    bool SerializeScene(const Scene& scene);

    bool DeserializeScene(Scene& outScene, const std::string& fileNameNoExt);
}