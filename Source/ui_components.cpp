#include <iostream>

#include "ImGUI/imgui.h"
#include "scene_manager.hpp"
#include "eventhandler.hpp"
#include "ui_components.hpp"

namespace UISystem
{
	static void Play()
	{
		SceneManager::RequestSceneSwitch("PrototypeLvl");
	}

	static void Pause() {}

	static void Restart()
	{
		SceneManager::RequestSceneReload();
	}

	static void Quit()
	{
		SceneManager::RequestSceneSwitch("MainMenu");
	}

	static void Exit()
	{
		SceneManager::QuitApplication();
	}
}

//void UIButtonRegister::bindFunction(FunctionKey key, CallbackF callF)
//{
//	_buttonReg[key] = callF;
//	std::cout << "Binded\n";
//}

void UIButtonRegister::init()
{
	EventHandler::Subscribe<UIButtonEvent>([this](IEvent* e)
		{
			auto uiEv = static_cast<UIButtonEvent*>(e); // Cast to access the fKey

			// This single function handles ALL UI buttons for this system
			switch (uiEv->fKey) 
			{
			case FunctionKey::PLAY_GAME:
				UISystem::Play();
				break;
			case FunctionKey::PAUSE_GAME:
				UISystem::Pause();
				break;
			case FunctionKey::RESTART_GAME:
				UISystem::Restart();
				break;
			case FunctionKey::QUIT_GAME:
				UISystem::Quit();
				break;
			case FunctionKey::EXIT_APP:
				UISystem::Exit();
				break;
			default:
				break;
			}
		});
}

void UIButtonRegister::handleMouseClick(FunctionKey key)
{
	std::cout << "Clicked\n";
	EventHandler::RaiseEvent<UIButtonEvent>(key);
	/*auto iterator = _buttonReg.find(key);

	if (iterator != _buttonReg.end() && iterator->second)
	{
		std::cout << "Function Called\n";
		iterator->second();
	}*/
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