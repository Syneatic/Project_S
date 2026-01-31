#pragma once
#include <string>
#include "ImGUI/imgui.h"

#include "component.hpp"

enum class Layer : uint32_t
{
    Nothing = 0,
    Player = 1 << 0,
    Environment = 1 << 1,
    Enemy = 1 << 2,
    Projectile = 1 << 3, //not for raycast for other object projectiles
};

enum class Tag
{
    Untagged,
    Player,
    Enemy,
    Platform,
};

inline const char* TagToString(Tag tag)
{
    switch (tag)
    {
        case Tag::Untagged: return "Untagged";
        case Tag::Player: return "Player";
        case Tag::Platform: return "Platform";
        default: return "Unknown";
    }
}

//abstract
struct Collider : Component
{
    uint32_t layerMask{ static_cast<uint32_t>(Layer::Nothing) };
    uint32_t collisionMask{ 0xFFFFFFFF };

    Tag tag{ Tag::Untagged };

    bool Has_Layer(Layer layer) const
    {
        return (layerMask & static_cast<uint32_t>(layer)) != 0;
    }

    void Add_Layer(Layer layer)
    {
        layerMask |= static_cast<uint32_t>(layer);
    }

    void Remove_Layer(Layer layer)
    {
        layerMask &= ~static_cast<uint32_t>(layer);
    }
    
    void Add_CollisionLayer(Layer layer)
    {
        collisionMask |= static_cast<uint32_t>(layer);
    }

    void Remove_CollisionLayer(Layer layer)
    {
        collisionMask &= ~static_cast<uint32_t>(layer);
    }

    bool CollidesWithLayer(Layer layer)const
    {
        return (collisionMask & static_cast<uint32_t>(layer)) != 0;
    }

    bool ShouldCollide(const Collider& other)const
    {
        return (collisionMask & other.layerMask) != 0;
    }


    void DrawLayerInInspector()
    {
        ImGui::Text("Tag:");
        ImGui::Text("Tag:");
        const char* tagNames[] = {
            "Untagged", "Player", "Enemy", "Platform"
        };

        int currentTag = static_cast<int>(tag);

        if (ImGui::Combo("##Tag", &currentTag, tagNames, 4))
        {
            tag = static_cast<Tag>(currentTag);
        }

        ImGui::Separator();
        ImGui::Text("I am (Layer):");

        bool isNothing = Has_Layer(Layer::Nothing);
        bool isPlayer = Has_Layer(Layer::Player);
        bool isEnemy = Has_Layer(Layer::Enemy);
        bool isEnvironment = Has_Layer(Layer::Environment);
        bool isProjectile = Has_Layer(Layer::Projectile);

        if (ImGui::Checkbox("Nothing", &isNothing))
        {
            if (isNothing) Add_Layer(Layer::Nothing);
            else Remove_Layer(Layer::Nothing);
        }

        if (ImGui::Checkbox("Player", &isPlayer))
        {
            if (isPlayer) Add_Layer(Layer::Player);
            else Remove_Layer(Layer::Player);
        }

        if (ImGui::Checkbox("Enemy", &isEnemy))
        {
            if (isEnemy) Add_Layer(Layer::Enemy);
            else Remove_Layer(Layer::Enemy);
        }

        if (ImGui::Checkbox("Environment", &isEnvironment))
        {
            if (isEnvironment) Add_Layer(Layer::Environment);
            else Remove_Layer(Layer::Environment);
        }


        if (ImGui::Checkbox("Projectile", &isProjectile))
        {
            if (isProjectile) Add_Layer(Layer::Projectile);
            else Remove_Layer(Layer::Projectile);
        }

        ImGui::Separator();

        ImGui::Text("I collide with (Mask):");

        bool collidesPlayer = CollidesWithLayer(Layer::Player);
        bool collidesEnemy = CollidesWithLayer(Layer::Enemy);
        bool collidesEnvironment = CollidesWithLayer(Layer::Environment);
        bool collidesProjectile = CollidesWithLayer(Layer::Projectile);


        if (ImGui::Checkbox("PlayerMask", &collidesPlayer))
        {
            if (collidesPlayer) Add_CollisionLayer(Layer::Player);
            else Remove_CollisionLayer(Layer::Player);
        }

        if (ImGui::Checkbox("EnemyMask", &collidesEnemy))
        {
            if (collidesEnemy) Add_CollisionLayer(Layer::Enemy);
            else Remove_CollisionLayer(Layer::Enemy);
        }

        if (ImGui::Checkbox("EnvironmentMask", &collidesEnvironment))
        {
            if (collidesEnvironment) Add_CollisionLayer(Layer::Environment);
            else Remove_CollisionLayer(Layer::Environment);
        }

        if (ImGui::Checkbox("ProjectileMask", &collidesProjectile))
        {
            if (collidesProjectile) Add_CollisionLayer(Layer::Projectile);
            else Remove_CollisionLayer(Layer::Projectile);
        }
    }
    protected:
    //tag
    //isTrigger
};

struct CircleCollider : Collider
{
    f32 radius{ 1.f };

    void DrawInInspector() override
    {
        DrawLayerInInspector();
        ImGui::Separator();
        ImGui::TextUnformatted("Size");
        ImGui::DragFloat("##circlecollider_radius", &radius, 0.1f);
    }

    void Serialize(Json::Value& outComp) const override
    {
        outComp["radius"] = radius;
        outComp["layerMask"] = layerMask;
        outComp["collisionMask"] = collisionMask;
        outComp["tag"] = static_cast<int>(tag);
    }

    void Deserialize(const Json::Value& compObj) override
    {
        if (compObj.isMember("radius") && compObj["radius"].isNumeric()) radius = compObj["radius"].asFloat();
        if (compObj.isMember("layerMask")) layerMask = compObj["layerMask"].asUInt();
        if (compObj.isMember("collisionMask")) collisionMask = compObj["collisionMask"].asUInt();
        if (compObj.isMember("tag")) tag = static_cast<Tag>(compObj["tag"].asInt());
    }

    const std::string name() const override { return "CircleCollider"; }

    CircleCollider() {};
};

struct BoxCollider : Collider
{
    float2 size{ 1.f,1.f };

    void DrawInInspector() override
    {
        DrawLayerInInspector();
        ImGui::Separator();
        ImGui::TextUnformatted("Size");
        ImGui::DragFloat2("##boxcollider_size", &size.x, 0.1f);
    }

    void Serialize(Json::Value& outComp) const override
    {
        outComp["size"] = WriteFloat2(size);
        outComp["layerMask"] = layerMask;
        outComp["collisionMask"] = collisionMask;
        outComp["tag"] = static_cast<int>(tag);
    }

    void Deserialize(const Json::Value& compObj) override
    {
        if (compObj.isMember("size")) ReadFloat2(compObj["size"], size);
        if (compObj.isMember("layerMask")) layerMask = compObj["layerMask"].asUInt();
        if (compObj.isMember("collisionMask")) collisionMask = compObj["collisionMask"].asUInt();
        if (compObj.isMember("tag")) tag = static_cast<Tag>(compObj["tag"].asInt());
    }

    const std::string name() const override { return "BoxCollider"; }
};