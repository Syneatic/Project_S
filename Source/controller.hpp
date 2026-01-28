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
    f32 maxspeed{ 200.f };
    f32 jumpHeight{ 20.f };
    float2 velocity{ 0.f, 0.f };
    f32 mass{ 50.f };

    f32 dt = (f32)AEFrameRateControllerGetFrameTime();

    void DrawInInspector() override
    {
        f32 total_width = ImGui::GetContentRegionAvail().x;
        f32 spacing = ImGui::GetStyle().ItemSpacing.x;

        ImGui::TextUnformatted("Max Speed");
        ImGui::DragFloat("##max_speed", &maxspeed, 0.1f, 0.f);

        ImGui::TextUnformatted("Jump Height");
        ImGui::DragFloat("##jumpHeight", &jumpHeight, 0.1f, 0.f);

        ImGui::TextUnformatted("Velocity");
        ImGui::PushItemWidth(((total_width - spacing) / 2) - 100.f);
        ImGui::DragFloat("##velocityX", &velocity.x, 0.f, -maxspeed, maxspeed);
        ImGui::SameLine();
        ImGui::DragFloat("##velocityY", &velocity.y, 0.f, -100.f, jumpHeight);
        ImGui::SameLine();
        if (ImGui::Button("Reset##Velocity"))
        {
            velocity.x = 0.0f;
            velocity.y = 0.0f;
        }
        ImGui::PopItemWidth();
    }

    void Serialize(Json::Value& outComp) const override
    {
        outComp["maxSpeed"] = maxspeed;
        outComp["velocty"] = WriteFloat2(velocity);
        outComp["jumpHeight"] = jumpHeight;
    }

    void Deserialize(const Json::Value& compObj) override
    {
        if (compObj.isMember("maxSpeed") && compObj["maxSpeed"].isNumeric())
            maxspeed = compObj["maxSpeed"].asFloat();
        if (compObj.isMember("velocity"))
            ReadFloat2(compObj["velocity"], velocity);
        if (compObj.isMember("jumpHeight") && compObj["jumpheight"].isNumeric())
            jumpHeight = compObj["jumpHeight"].asFloat();
    }

    void OnStart() override
    {
    } 

    bool isGravityOn = true;
    bool OnGround = false;
    f32 start = 200.f;
    f32 end = 0.f;
    f32 ground{};
    f32 t = 0.f;

    void OnUpdate() override
    {
        GameObject& owner = *_owner;
        Transform& trans = *owner.GetComponent<Transform>();

        if (AEInputCheckCurr(AEVK_A))
        {
            trans.position.x -= 200.f * dt;
        }
        if (AEInputCheckCurr(AEVK_D))
        {
            trans.position.x += 200.f * dt;
        }

        //Once if space is pressed once
        if (AEInputCheckTriggered(AEVK_SPACE) && OnGround)
        {
            //Set the space bar velocity to true
            //Check if the player reach the height (dt)
            isGravityOn = false;
            OnGround = false;
            t = 0.f;
        }

        //Make the player able to jump
        if (!OnGround && !isGravityOn)
        {
            const float jumpDuration = 1.f;
            //Check the total number it travels up
            //Constantly making the player go up
            /*trans.position.y = (currJumpHeight += 100.f * dt);*/
            if (t <= jumpDuration)
            {
                t += dt;
                float alpha = t / jumpDuration;
                velocity.y = start + alpha * (end - start);
                std::cout << velocity.y << std::endl;
            }
            else
            {
                isGravityOn = true;
                std::cout << "Gravity back on" << std::endl;
            }
        }

        //Make the player go down (til it reaches the ideal location)
        if (!OnGround && isGravityOn)
        {
            const float gravity = 450.F;
            velocity.y -= gravity * dt;
        }

        trans.position.y += velocity.y * dt;

        if (trans.position.y <= ground)
        {
            //std::cout << "Touched the ground" << std::endl;
            velocity.y = 0;
            isGravityOn = false;
            OnGround = true;
            trans.position.y = ground;
            //t = 0.f;
        }
    }

    void OnDestroy() override
    {

    }

    const std::string name() const override { return "PlayerController"; }
};