#include "ImGUI/imgui.h"

#include "transform_component.hpp"
#include "json_parser_helper.hpp"

//should only be available in debug mode
void Transform::DrawInInspector()
{
    ImGui::TextUnformatted("Position");
    ImGui::DragFloat2("##transform_position", &position.x, 0.05f);
    ImGui::SameLine();
    if (ImGui::Button("Reset##Pos"))
    {
        position.x = 0.0f;
        position.y = 0.0f;
    }
    
    ImGui::TextUnformatted("Scale");
    ImGui::DragFloat2("##transform_scale", &scale.x, 0.05f);
    ImGui::SameLine();
    if (ImGui::Button("Reset##Scale"))
    {
        scale.x = 1.0f;
        scale.y = 1.0f;
    }
    
    ImGui::TextUnformatted("Rotation");
    ImGui::DragFloat("##transform_rotation", &rotation, 0.1f);
    ImGui::SameLine();
    if (ImGui::Button("Reset##Rot"))
    {
        rotation = 0.0f;
    }
}

void Transform::Serialize(Json::Value& outComp) const
{
    outComp["position"] = WriteFloat2(position);
    outComp["scale"] = WriteFloat2(scale);
    outComp["rotation"] = rotation;
}

void Transform::Deserialize(const Json::Value& compObj)
{
    if (compObj.isMember("position")) ReadFloat2(compObj["position"], position);
    if (compObj.isMember("scale"))    ReadFloat2(compObj["scale"], scale);
    if (compObj.isMember("rotation") && compObj["rotation"].isNumeric())
        rotation = compObj["rotation"].asFloat();
}
