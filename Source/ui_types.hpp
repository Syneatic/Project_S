#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include "component.hpp"
#include "math.hpp"
#include "AEEngine.h"

// Enum class ID for all possible button functions.
enum class FunctionKey
{
	PLAY_GAME,
	PAUSE_GAME,
	RESTART_GAME,
	SETTINGS_TOGGLE,
	MUSIC_INC, MUSIC_DEC,
	SFX_INC, SFX_DEC,
	SAVE_GAME, LOAD_GAME,
	CREDITS_TOGGLE,
	QUIT_GAME,
	EXIT_APP,
	COUNT
};

using CallbackF = void(*)(); // Identifier for void pointer function with void param.
using ButtonRegister = std::unordered_map<FunctionKey, CallbackF>; // Identifier for unordered_map with FunctionKey & CallbackF.

// UIButtonRegister for handling ButtonRegister assignment & CallbackF logic.
class UIButtonRegister
{
public:
	// Singleton.
	static UIButtonRegister& Instance()
	{
		static UIButtonRegister instance;
		return instance;
	}

	UIButtonRegister(const UIButtonRegister&) = delete;
	UIButtonRegister& operator=(const UIButtonRegister&) = delete;
	UIButtonRegister(UIButtonRegister&&) = delete;
	UIButtonRegister& operator=(UIButtonRegister&&) = delete;

	void bindFunction(FunctionKey key, CallbackF callF)
	{
		buttonReg[key] = callF;
	}

	void handleMouseClick(FunctionKey key)
	{
		auto iterator = buttonReg.find(key);
		if (iterator != buttonReg.end() && iterator->second)
			iterator->second();
	}

private:
	ButtonRegister buttonReg;
	UIButtonRegister() {}
};

void BindButtonFunctions(UIButtonRegister& bReg);

// Display component to attach image or custom texture.
struct Display : Behaviour
{
	AEGfxTexture* texture{ nullptr };

	void DrawInInspector() override
	{
		ImGui::TextUnformatted("Texture");
	}

	void OnStart() override {}
	void OnUpdate() override {}
	void OnDestroy() override {}

	const std::string name() const override { return "Display"; }
};

// Text component to assign text on screen.
struct Text : Behaviour
{
	int fontSize;
	std::string str;

	void DrawInInspector() override
	{
		char cStr[128]; strcpy_s(cStr, str.c_str());
		ImGui::InputText("Text##Text", cStr, IM_ARRAYSIZE(cStr));
		str = cStr;
	}

	void Serialize(Json::Value& outComp) const override
	{
		outComp["string"] = str;
	}

	void Deserialize(const Json::Value& compObj) override
	{
		if (compObj.isMember("string"))
			str = compObj["string"].asString();
	}

	void OnStart() override {}
	void OnUpdate() override {}
	void OnDestroy() override {}

	const std::string name() const override { return "Text"; }
};

static char const* _buttonNames[]
{
	"GamePlay", "GamePause", "GameRestart", "ToggleSettings", "MusicUp", "MusicDown",
	"SfxUp", "SfxDown", "GameSave", "GameLoad", "ToggleCredits", "GameQuit", "AppExit"
};

void Hover_Logic(GameObject& button, UIButtonRegister& bReg);

// Button Component to assign function callback.
struct Button : Behaviour
{
	FunctionKey fKey;

	void DrawInInspector() override
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

	void Serialize(Json::Value& outComp) const override
	{
		outComp["buttonFunctionId"] = static_cast<int>(fKey);
	}

	void Deserialize(const Json::Value& compObj) override
	{
		if (compObj.isMember("buttonFunctionId"))
			fKey = static_cast<FunctionKey>(compObj["buttonFunctionId"].asInt());
	}

	void OnStart() override {}
	void OnUpdate() override
	{
		GameObject& owner = *_owner;
		Hover_Logic(owner, UIButtonRegister::Instance());
	}
	void OnDestroy() override {}

	const std::string name() const override { return "Button"; }
};
