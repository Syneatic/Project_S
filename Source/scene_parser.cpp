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

        //skip unknown components
        //if (type != "Transform" && type != "CircleCollider" && type != "BoxCollider")
        //    return false;

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

        if (type == "Transform")
        {
            Transform t{};
            t.Deserialize(compObj);
            go.AddComponent<Transform>(t);
        }
        else if (type == "CircleCollider")
        {
            CircleCollider c{};
            c.Deserialize(compObj);
            go.AddComponent<CircleCollider>(c);
        }
        else if (type == "BoxCollider")
        {
            BoxCollider b{};
            b.Deserialize(compObj);
            go.AddComponent<BoxCollider>(b);
        }
        else if (type == "SpriteRenderer")
        {
            SpriteRenderer r{};
            r.Deserialize(compObj);
            go.AddComponent<SpriteRenderer>(r);
        }
        else if (type == "TextRenderer")
        {
            TextRenderer r{};
            r.Deserialize(compObj);
            go.AddComponent<TextRenderer>(r);
        }
        else if (type == "PlayerController")
        {
            PlayerController pc{};
            pc.Deserialize(compObj);
            go.AddComponent<PlayerController>(pc);
        }
        else if (type == "RockController")
        {
            RockController rc{};
            rc.Deserialize(compObj);
            go.AddComponent<RockController>(rc);
        }
        /*else if (type == "Text")
        {
            Text str{};
            str.Deserialize(compObj);
            go.AddComponent<Text>(str);
        }*/
        else if (type == "ParticleEmitter")
        {
            ParticleEmitter ept{};
            ept.Deserialize(compObj);
            go.AddComponent<ParticleEmitter>(ept);
        }
        else if (type == "Button")
        {
            Button b{};
            b.Deserialize(compObj);
            go.AddComponent<Button>(b);
        }
        else if (type == "RigidBody")
        {
            RigidBody rb{};
            rb.Deserialize(compObj);
            go.AddComponent<RigidBody>(rb);
        }
        //AUDIO COMPONENTS
        else if (type == "AudioEmitter")
        {
            AudioEmitter& ae = go.AddComponent<AudioEmitter>();
            ae.Deserialize(compObj);
        }
        else if (type == "AudioListener")
        {
            AudioListener& al = go.AddComponent<AudioListener>();
            al.Deserialize(compObj);
        }
        else if (type == "NoiseSource")
        {
            NoiseSource& ns = go.AddComponent<NoiseSource>();
            ns.Deserialize(compObj);
        }
        else if (type == "Camera")
        {
            MainCamera& cm = go.AddComponent<MainCamera>();
            cm.Deserialize(compObj);
        }
    }

    // ===== GameObject Serialization =====
    Json::Value SerializeGameObject(const GameObject& go)
    {
        Json::Value obj(Json::objectValue);
        obj["name"] = go.name();
        obj["active"] = go.active();

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