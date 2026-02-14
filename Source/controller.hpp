#include "ImGUI/imgui.h"
#include "AEEngine.h"

#include <string>

#include "component.hpp"
#include "gameobject.hpp"
#include "physics_component.hpp"
#include "math.hpp"
#include <iostream>

//abstract
struct Controller : Behaviour
{
    //max speed
    //velocity
    //jump height
    //collidertype
};

struct PlayerController : Controller
{
    f32 maxSpeed = 10.f;
    f32 jumpHeight = 500.f;

    f32 dt = (f32)AEFrameRateControllerGetFrameTime();

    void DrawInInspector() override
    {

        ImGui::TextUnformatted("Max Speed");
        ImGui::DragFloat("##max_speed", &maxSpeed, 0.1f);

        ImGui::TextUnformatted("Jump Height");
        ImGui::DragFloat("##jumpHeight", &jumpHeight, 0.1f);
    }

    void Serialize(Json::Value& outComp) const override
    {
        outComp["max_speed"] = maxSpeed;
        outComp["jumpHeight"] = jumpHeight;
    }

    void Deserialize(const Json::Value& compObj) override
    {
        if (compObj.isMember("max_speed") && compObj["max_speed"].isNumeric())
            maxSpeed = compObj["max_speed"].asFloat();
        if (compObj.isMember("jumpHeight") && compObj["jumpHeight"].isNumeric())
            jumpHeight = compObj["jumpHeight"].asFloat();
    }

    void OnStart() override
    {

    } 

    //f32 end = 0.f;
    //f32 t = 0.f;

    void OnUpdate() override
    {
        GameObject& owner = *_owner;
        Transform* trans = owner.GetComponent<Transform>();
        RigidBody* rb = owner.GetComponent<RigidBody>();
        
        if (!trans || !rb) return;

        rb->velocity.x = 0.f;

        if (AEInputCheckCurr(AEVK_A))
        {
            rb->velocity.x = -maxSpeed;
        }
        if (AEInputCheckCurr(AEVK_D))
        {
            rb->velocity.x = maxSpeed;
        }

        //Once if space is pressed once
        if (AEInputCheckTriggered(AEVK_SPACE) && rb->Is_Grounded)
        {           
            //Set the space bar velocity to true
            //Check if the player reach the height (dt)
            //rb.Affected_By_Gravity = false;
            rb->velocity.y = jumpHeight;
            rb->Is_Grounded = false;
            //t = 0.f;
        }
    }

    void OnDestroy() override
    {

    }

    const std::string name() const override { return "PlayerController"; }
};