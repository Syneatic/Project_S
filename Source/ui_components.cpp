#include <iostream>

#include "ImGUI/imgui.h"

#include "ui_components.hpp"


void UIButtonRegister::bindFunction(FunctionKey key, CallbackF callF)
{
	_buttonReg[key] = callF;
	std::cout << "Binded\n";
}

void UIButtonRegister::handleMouseClick(FunctionKey key)
{
	std::cout << "Clicked\n";
	auto iterator = _buttonReg.find(key);

	if (iterator != _buttonReg.end() && iterator->second)
	{
		std::cout << "Function Called\n";
		iterator->second();
	}
}




void Display::DrawInInspector()
{
	ImGui::TextUnformatted("Texture");
}

void Display::OnStart()  {}
void Display::OnUpdate() {}
void Display::OnDestroy(){}



void Button::DrawInInspector()
{
	if (ImGui::BeginCombo("ButtonMode", "SelectButton"))
	{
		for (int i = 0; i < static_cast<int>(FunctionKey::COUNT); i++)
		{
			bool is_selected = (i == static_cast<int>(fKey));

			if (ImGui::Selectable(_buttonNames[i], is_selected))
				fKey = static_cast<FunctionKey>(i);

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}
}

void Button::Serialize(Json::Value& outComp) const
{
	outComp["buttonFunctionId"] = static_cast<int>(fKey);
}

void Button::Deserialize(const Json::Value& compObj)
{
	if (compObj.isMember("buttonFunctionId"))
		fKey = static_cast<FunctionKey>(compObj["buttonFunctionId"].asInt());
}

void Button::OnStart() {}
void Button::OnUpdate()
{
	//GameObject& owner = *_owner;
	UISystem::Hover_Logic(gameObject(), UIButtonRegister::Instance());
}
void Button::OnDestroy(){}