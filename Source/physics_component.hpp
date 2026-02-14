#pragma once 
#include "component.hpp"
#include "math.hpp"
#include "ImGUI/imgui.h"

enum class Layer : uint32_t
{
    Nothing = 0,
    Player = 1 << 0,
    Environment = 1 << 1,
    Enemy = 1 << 2,
    Projectile = 1 << 3, //not for raycast for other object projectiles
};

inline const char* LayerToString(Layer layer)
{
    switch (layer)
    {
    case Layer::Nothing: return "Nothing";
    case Layer::Player: return "Player";
    case Layer::Enemy: return "Enemy";
    case Layer::Environment: return "Environment";
    case Layer::Projectile: return "Projectile";
    default: return "Untagged";
    }
}

//abstract
struct Collider : Component
{
    uint32_t layerMask{ static_cast<uint32_t>(Layer::Nothing) };
    uint32_t collisionMask{ 0xFFFFFFFF };


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


        ImGui::Separator();
        ImGui::Text("I am (Layer):");

        const char* layerNames[] = {
        "Nothing", "Player", "Environment", "Enemy", "Projectile"
        };

        Layer layers[] = {
        Layer::Nothing, Layer::Player, Layer::Environment,
        Layer::Enemy, Layer::Projectile
        };

        int currentLayer = 0;
        for (int i = 0; i < 5; i++)
        {
            if (Has_Layer(layers[i]))
            {
                currentLayer = i;
                break;
            }
        }

        if (ImGui::Combo("##Layer", &currentLayer, layerNames, 5))
        {
            layerMask = static_cast<uint32_t>(layers[currentLayer]);
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
    }

    void Deserialize(const Json::Value& compObj) override
    {
        if (compObj.isMember("radius") && compObj["radius"].isNumeric()) radius = compObj["radius"].asFloat();
        if (compObj.isMember("layerMask")) layerMask = compObj["layerMask"].asUInt();
        if (compObj.isMember("collisionMask")) collisionMask = compObj["collisionMask"].asUInt();
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
    }

    void Deserialize(const Json::Value& compObj) override
    {
        if (compObj.isMember("size")) ReadFloat2(compObj["size"], size);
        if (compObj.isMember("layerMask")) layerMask = compObj["layerMask"].asUInt();
        if (compObj.isMember("collisionMask")) collisionMask = compObj["collisionMask"].asUInt();
    }

    const std::string name() const override { return "BoxCollider"; }
};

struct RigidBody :Component 
{ 
	bool Affected_By_Gravity{ false };
	bool Is_Static{ false };
	bool Is_Grounded{ false };
	float gravity{ 9.8f };
	float terminalVelocity{ 12.0f };
	float2 velocity{ 0.0f,0.0f };


	void DrawInInspector()override 
	{ 
		ImGui::Checkbox("Is Static", &Is_Static);
		ImGui::Checkbox("Affected By Gravity", &Affected_By_Gravity);
		ImGui::Checkbox("Grounded", &Is_Grounded);
		ImGui::DragFloat("Gravity", &gravity, 0.1f);
		ImGui::DragFloat2("Velocity", &velocity.x, 0.1f);
	} 
	void Serialize(Json::Value& outComp) const override
	{
		outComp["Is Static"] = Is_Static;
		outComp["Affected By Gravity"] = Affected_By_Gravity;
		outComp["Grounded"] = Is_Grounded;
		outComp["Gravity"] = gravity;
		outComp["Velocity"] = velocity.x;
	}

	void Deserialize(const Json::Value& compObj) override
	{
		if (compObj.isMember("Is Static") && compObj["Is Static"].isBool())
			Is_Static = compObj["Is Static"].asBool();
		if (compObj.isMember("Affected By Gravity") && compObj["Affected By Gravity"].isBool())
			Affected_By_Gravity = compObj["Affected By Gravity"].asBool();
		if (compObj.isMember("Grounded") && compObj["Grounded"].isBool())
			Is_Grounded = compObj["Grounded"].asBool();
		if (compObj.isMember("Gravity") && compObj["Gravity"].isNumeric())
			gravity = compObj["Gravity"].asFloat();
		if (compObj.isMember("Velocity") && compObj["Velocity"].isNumeric())
			velocity.x = compObj["Velocity"].asFloat();
	}
	void Clear_Forces() 
	{ 
		if (Is_Static) 
		{ 
			velocity = float2::zero(); 
		} 
	} 
	const std::string name() const override { return "RigidBody"; }
};