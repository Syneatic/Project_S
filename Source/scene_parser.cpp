#include "math.hpp"

#include "scene.hpp"
#include "gameobject.hpp"

#include "components.hpp"

namespace SceneIO
{
    static std::string defaultPath = "Assets/Scene/";

    // ===== Component Serialization =====
    bool SerializeComponent(const Component& c, Json::Value& outComp)
    {
        const std::string type = c.name();

        outComp = Json::Value(Json::objectValue);
        outComp["type"] = type;

        c.Serialize(outComp);

        return true;
    }

    void DeserializeComponent(GameObject& go, const Json::Value& compObj)
    {
        if (!compObj.isObject()) return;
        if (!compObj.isMember("type") || !compObj["type"].isString()) return;

        const std::string type = compObj["type"].asString();

        if (type == "CircleCollider")
        {
            auto& c = go.AddComponent<CircleCollider>();
			c.Deserialize(compObj);
        }
        else if (type == "BoxCollider")
        {
            auto& c = go.AddComponent<BoxCollider>();
            c.Deserialize(compObj);
        }
        else if (type == "SpriteRenderer")
        { 
            auto& c = go.AddComponent<SpriteRenderer>();
            c.Deserialize(compObj);
        }
        else if (type == "TextRenderer")
        {
            auto& c = go.AddComponent<TextRenderer>();
            c.Deserialize(compObj);
        }
        else if (type == "PlayerController")
        {  
            auto& c = go.AddComponent<PlayerController>();
            c.Deserialize(compObj);
        }
        else if (type == "RockController")
        {
            auto& c = go.AddComponent<RockController>();
            c.Deserialize(compObj);
        }
        else if (type == "EnemyController")
        {
            auto& c = go.AddComponent<EnemyController>();
            c.Deserialize(compObj);
        }
        else if (type == "ParticleEmitter")
        { 
            auto& c = go.AddComponent<ParticleEmitter>();
            c.Deserialize(compObj);
        }
        else if (type == "Button")
        {  
            auto& c = go.AddComponent<Button>();
            c.Deserialize(compObj);
        }
        else if (type == "RigidBody")
        {   
            auto& c = go.AddComponent<RigidBody>();
            c.Deserialize(compObj);
        }
        //AUDIO COMPONENTS
        else if (type == "AudioEmitter")
        {
            auto& ae = go.AddComponent<AudioEmitter>();
            ae.Deserialize(compObj);
        }
        else if (type == "AudioListener")
        {
            auto& al = go.AddComponent<AudioListener>();
            al.Deserialize(compObj);
        }
        else if (type == "NoiseSource")
        {
            auto& ns = go.AddComponent<NoiseSource>();
            ns.Deserialize(compObj);
        }
        else if (type == "Camera")
        {
            auto& cm = go.AddComponent<MainCamera>();
            cm.Deserialize(compObj);
        }
    }

    // ===== GameObject Serialization =====
    Json::Value SerializeGameObject(const GameObject& go)
    {
		const Transform& transform = go.transform();
        Json::Value obj(Json::objectValue);
        obj["name"] = go.name();
        obj["active"] = go.active();

        //serialize transform
        obj["position"] = WriteFloat2(transform.position);
        obj["scale"] = WriteFloat2(transform.scale);
        obj["rotation"] = transform.rotation;

        // components
        Json::Value comps(Json::arrayValue);
        const auto& cmap = go.componentMap();
        for (auto it = cmap.begin(); it != cmap.end(); ++it)
        {
            const std::unique_ptr<Component>& cptr = it->second;
            if (!cptr) continue;

            Json::Value comp;
            if (SerializeComponent(*cptr, comp))
                comps.append(comp);
        }
        obj["components"] = comps;

        // children
        Json::Value children(Json::arrayValue);
        for (const auto& child : go.children())
        {
            if (!child) continue;
            children.append(SerializeGameObject(*child));
        }
        obj["children"] = children;

        return obj;
    }

    std::unique_ptr<GameObject> DeserializeGameObject(const Json::Value& obj)
    {
        if (!obj.isObject()) return nullptr;
        if (!obj.isMember("name") || !obj["name"].isString()) return nullptr;

        auto go = std::make_unique<GameObject>(obj["name"].asString());
        auto& transform = go.get()->transform();
        if (obj.isMember("active") && obj["active"].isBool())
            go->active(obj["active"].asBool());

        //deserialize transform
        if (obj.isMember("position")) ReadFloat2(obj["position"], transform.position);
        if (obj.isMember("scale"))    ReadFloat2(obj["scale"], transform.scale);
        if (obj.isMember("rotation") && obj["rotation"].isNumeric())
            transform.rotation = obj["rotation"].asFloat();

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
    bool SerializeScene(const Scene& scene)
    {
        namespace fs = std::filesystem;
        try { fs::create_directories(defaultPath); }
        catch (...) {}

        Json::Value root(Json::objectValue);
        root["name"] = scene.name();

        Json::Value gos(Json::arrayValue);
        auto& list = scene.gameObjectList();
        for (const auto& g : list)
        {
            if (!g) continue;
            gos.append(SerializeGameObject(*g));
        }
        root["gameObjects"] = gos;

        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

#ifdef _DEBUG
        //for debug
        const std::string dpath = "../../Assets/Scene/" + scene.name() + ".scene";
        std::ofstream dout(dpath, std::ios::binary);
        if (!dout)
        {
            std::cout << "UNABLE TO CREATE FILE\n";
            return false;
        }

        if (writer->write(root, &dout) != 0)
        {
            std::cout << "SCENE SERIALIZE FAILED\n";
        }
#endif

        const std::string path = "Assets/Scene/" + scene.name() + ".scene";
        std::ofstream out(path, std::ios::binary);
        if (!out)
        {
            std::cout << "UNABLE TO CREATE FILE\n";
            return false;
        }

        return (writer->write(root, &out) == 0);
    }

    bool DeserializeScene(Scene& outScene, const std::string& fileNameNoExt)
    {
#ifdef _DEBUG
        const std::string path = "../../Assets/Scene/" + fileNameNoExt + ".scene";
#else
        const std::string path = "Assets/Scene/" + fileNameNoExt + ".scene";
#endif
        std::ifstream in(path, std::ios::binary);
        if (!in) return false;

        Json::CharReaderBuilder rbuilder;
        std::string errs;
        Json::Value root;

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