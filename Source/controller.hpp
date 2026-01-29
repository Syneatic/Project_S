#include "ImGUI/imgui.h"
#include "AEEngine.h"

#include <string>

#include "component.hpp"
#include "gameobject.hpp"
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
    f32 maxSpeed = 200.f;
    f32 jumpHeight = 100.f;

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

    f32 end = 0.f;
    f32 t = 0.f;

    void OnUpdate() override
    {
        GameObject& owner = *_owner;
        Transform& trans = *owner.GetComponent<Transform>();
        RigidBody& rb = *owner.GetComponent<RigidBody>();

        if (AEInputCheckCurr(AEVK_A))
        {
            trans.position.x -= 200.f * dt;
        }
        if (AEInputCheckCurr(AEVK_D))
        {
            trans.position.x += 200.f * dt;
        }

        //Once if space is pressed once
        if (AEInputCheckTriggered(AEVK_SPACE) && rb.Is_Grounded)
        {
            //Set the space bar velocity to true
            //Check if the player reach the height (dt)
            rb.Affected_By_Gravity = false;
            rb.Is_Grounded = false;
            t = 0.f;
        }

        //Make the player able to jump
        if (!rb.Is_Grounded && !rb.Affected_By_Gravity)
        {
            const float jumpDuration = 1.f;
            //Check the total number it travels up
            //Constantly making the player go up
            /*trans.position.y = (currJumpHeight += 100.f * dt);*/
            if (t <= jumpDuration)
            {
                t += dt;
                float alpha = t / jumpDuration;
                rb.velocity.y = jumpHeight + alpha * (end - jumpHeight);
                std::cout << rb.velocity.y << std::endl;
            }
            else
            {
                rb.Affected_By_Gravity = true;
                std::cout << "Gravity back on" << std::endl;
            }
        }

        //if (trans.position.y <= rb.Is_Grounded)
        //{
        //    //std::cout << "Touched the ground" << std::endl;
        //    rb.velocity.y = 0;
        //    rb.Affected_By_Gravity = false;
        //    rb.Is_Grounded = true;
        //    //t = 0.f;
        //}
    }

    void OnDestroy() override
    {

    }

    const std::string name() const override { return "PlayerController"; }
};