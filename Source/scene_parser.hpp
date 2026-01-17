#pragma once
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "json.h"

#include "scene.hpp"
#include "gameobject.hpp"
#include "component.hpp"
#include "math.hpp"

// parse a scene object into a scene file
// read it back from the scene file into object on load

namespace SceneIO
{
	using namespace Json;
	static std::string defaultPath = "OUT/";

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
        if (type != "Transform" && type != "CircleCollider" && type != "BoxCollider")
            return false;

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

        return true;
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
}