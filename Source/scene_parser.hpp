#pragma once
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "json.h"

#include "scene.hpp"
#include "gameobject.hpp"
#include "component.hpp"
#include "render_components.hpp"
#include "collider_components.hpp"
#include "controller.hpp"
#include "math.hpp"
#include "ui_types.hpp"

// parse a scene object into a scene file
// read it back from the scene file into object on load

namespace SceneIO
{
	using namespace Json;
	static std::string defaultPath = "Scene/";

	// ===== Helper Functions =====
	inline Value WriteFloat2(const float2& v)
	{
		Value arr(Json::arrayValue);
		arr.append(v.x);
		arr.append(v.y);
		return arr;
	}

	inline void ReadFloat2(const Value& arr, float2& out)
	{
		if (!arr.isArray() || arr.size() != 2) return;
		if (!arr[0].isNumeric() || !arr[1].isNumeric()) return;
		out.x = arr[0].asFloat();
		out.y = arr[1].asFloat();
	}

	// ===== Component Serialization =====
    inline bool SerializeComponent(const Component& c, Value& outComp)
    {
        const std::string type = c.name();

        //skip unknown components
        //if (type != "Transform" && type != "CircleCollider" && type != "BoxCollider")
        //    return false;

        outComp = Value(objectValue);
        outComp["type"] = type;

        if (type == "Transform")
        {
            auto const& t = static_cast<Transform const&>(c);
            outComp["position"] = WriteFloat2(t.position);
            outComp["scale"] = WriteFloat2(t.scale);
            outComp["rotation"] = t.rotation;
        }
        else if (type == "CircleCollider")
        {
            auto const& cc = static_cast<CircleCollider const&>(c);
            outComp["radius"] = cc.radius;
        }
        else if (type == "BoxCollider")
        {
            auto const& bc = static_cast<BoxCollider const&>(c);
            outComp["size"] = WriteFloat2(bc.size);
        }
        else if (type == "PlayerController")
        {
            auto const& pc = static_cast<PlayerController const&>(c);
            outComp["maxSpeed"] = pc.maxspeed;
            outComp["velocty"] = WriteFloat2(pc.velocity);
            outComp["gravity"] = pc.gravity;
            outComp["jumpHeight"] = pc.jumpHeight;
            outComp["drag"] = pc.drag;
        }
        else if (type == "Text")
        {
            auto const& str = static_cast<Text const&>(c);
            outComp["string"] = str.str;
        }

        return true;
    }

    inline void DeserializeComponent(GameObject& go, const Value& compObj)
    {
        if (!compObj.isObject()) return;
        if (!compObj.isMember("type") || !compObj["type"].isString()) return;

        const std::string type = compObj["type"].asString();

        if (type == "Transform")
        {
            Transform t{};
            if (compObj.isMember("position")) ReadFloat2(compObj["position"], t.position);
            if (compObj.isMember("scale"))    ReadFloat2(compObj["scale"], t.scale);
            if (compObj.isMember("rotation") && compObj["rotation"].isNumeric())
                t.rotation = compObj["rotation"].asFloat();
            go.AddComponent<Transform>(t);
        }
        else if (type == "CircleCollider")
        {
            CircleCollider c{};
            if (compObj.isMember("radius") && compObj["radius"].isNumeric())
                c.radius = compObj["radius"].asFloat();
            go.AddComponent<CircleCollider>(c);
        }
        else if (type == "BoxCollider")
        {
            BoxCollider b{};
            if (compObj.isMember("size")) ReadFloat2(compObj["size"], b.size);
            go.AddComponent<BoxCollider>(b);
        }
        else if (type == "SpriteRenderer")
        {
            SpriteRenderer r{};
            go.AddComponent<SpriteRenderer>(r);
        }
        else if (type == "PlayerController")
        {
            PlayerController pc{};
            if (compObj.isMember("maxSpeed") && compObj["maxSpeed"].isNumeric())
                pc.maxspeed = compObj["maxSpeed"].asFloat();
            if (compObj.isMember("velocity"))
                ReadFloat2(compObj["velocity"], pc.velocity);
            if (compObj.isMember("gravity") && compObj["gravity"].isNumeric())
                pc.gravity = compObj["gravity"].asFloat();
            if (compObj.isMember("jumpHeight") && compObj["jumpheight"].isNumeric())
                pc.jumpHeight = compObj["jumpHeight"].asFloat();
            if (compObj.isMember("drag") && compObj["drag"].isNumeric())
                pc.drag = compObj["drag"].asFloat();
            go.AddComponent<PlayerController>(pc);
        }
        else if (type == "Text")
        {
            Text str{};
            if (compObj.isMember("string"))
                str.str = compObj["string"].asString();
            go.AddComponent<Text>(str);
        }
    }

    // ===== GameObject Serialization =====
    inline Value SerializeGameObject(const GameObject& go)
    {
        Value obj(objectValue);
        obj["name"] = go.name();
        obj["active"] = go.active();

        // components
        Value comps(arrayValue);
        const auto& cmap = go.componentMap();
        for (auto it = cmap.begin(); it != cmap.end(); ++it)
        {
            const std::unique_ptr<Component>& cptr = it->second;
            if (!cptr) continue;

            Value comp;
            if (SerializeComponent(*cptr, comp))
                comps.append(comp);
        }
        obj["components"] = comps;

        // children
        Value children(arrayValue);
        for (const auto& child : go.children())
        {
            if (!child) continue;
            children.append(SerializeGameObject(*child));
        }
        obj["children"] = children;

        return obj;
    }
    
    inline std::unique_ptr<GameObject> DeserializeGameObject(const Value& obj)
    {
        if (!obj.isObject()) return nullptr;
        if (!obj.isMember("name") || !obj["name"].isString()) return nullptr;

        auto go = std::make_unique<GameObject>(obj["name"].asString());

        if (obj.isMember("active") && obj["active"].isBool())
            go->active(obj["active"].asBool());

        if (obj.isMember("components") && obj["components"].isArray())
        {
            for (const auto& c : obj["components"])
                DeserializeComponent(*go, c);
        }

        if (obj.isMember("children") && obj["children"].isArray())
        {
            for (const auto& ch : obj["children"])
            {
                auto child = DeserializeGameObject(ch);
                if (child) go->AddChild(std::move(child));
            }
        }

        return go;
    }

    // ===== Scene Serialization =====
    inline bool SerializeScene(const Scene& scene)
    {
        
        namespace fs = std::filesystem;
        try { fs::create_directories(defaultPath); }
        catch (...) {}

        Value root(objectValue);
        root["name"] = scene.name();

        Value gos(arrayValue);
        auto& list = scene.gameObjectList();
        for (const auto& g : list)
        {
            if (!g) continue;
            gos.append(SerializeGameObject(*g));
        }
        root["gameObjects"] = gos;

        const std::string path = defaultPath + scene.name() + ".scene";
        std::ofstream out(path, std::ios::binary);
        if (!out)
        {
            std::cout << "UNABLE TO CREATE FILE\n";
            return false;
        }

        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        return (writer->write(root, &out) == 0);
    }

    inline bool DeserializeScene(Scene& outScene, const std::string& fileNameNoExt)
    {
        const std::string path = defaultPath + fileNameNoExt + ".scene";
        std::ifstream in(path, std::ios::binary);
        if (!in) return false;

        CharReaderBuilder rbuilder;
        std::string errs;
        Value root;

        if (!parseFromStream(rbuilder, in, &root, &errs))
            return false;

        if (root.isMember("name") && root["name"].isString())
            outScene.name(root["name"].asString());

        auto& list = outScene.gameObjectList();
        list.clear();

        if (root.isMember("gameObjects") && root["gameObjects"].isArray())
        {
            for (const auto& g : root["gameObjects"])
            {
                auto go = DeserializeGameObject(g);
                if (go) list.emplace_back(std::move(go));
            }
        }

        return true;
    }
}