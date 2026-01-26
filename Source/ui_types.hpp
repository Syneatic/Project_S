#pragma once
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

const FunctionKey functionList[static_cast<size_t>(FunctionKey::COUNT)]
{
	FunctionKey::PLAY_GAME,
	FunctionKey::PAUSE_GAME,
	FunctionKey::RESTART_GAME,
	FunctionKey::SETTINGS_TOGGLE,
	FunctionKey::MUSIC_INC, FunctionKey::MUSIC_DEC,
	FunctionKey::SFX_INC, FunctionKey::SFX_DEC,
	FunctionKey::SAVE_GAME, FunctionKey::LOAD_GAME,
	FunctionKey::CREDITS_TOGGLE,
	FunctionKey::QUIT_GAME,
	FunctionKey::EXIT_APP
};

using CallbackF = void(*)(); // Identifier for void pointer function with void param.
using ButtonRegister = std::unordered_map<FunctionKey, CallbackF>; // Identifier for unordered_map with FunctionKey & CallbackF.

// UIButtonRegister for handling ButtonRegister assignment & CallbackF logic.
class UIButtonRegister
{
public:
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

static char const* ReturnButtonName(FunctionKey k)
{
	switch (k)
	{
	case FunctionKey::PLAY_GAME:	return "GamePlay";
	case FunctionKey::PAUSE_GAME:	return "GamePause";
	case FunctionKey::RESTART_GAME:	return "GameRestart";
	case FunctionKey::SETTINGS_TOGGLE:	return "ToggleSettings";
	case FunctionKey::MUSIC_INC:	return "MusicUp";
	case FunctionKey::MUSIC_DEC:	return "MusicDown";
	case FunctionKey::SFX_INC:		return "SfxUp";
	case FunctionKey::SFX_DEC:		return "SfxDown";
	case FunctionKey::SAVE_GAME:	return "GameSave";
	case FunctionKey::LOAD_GAME:	return "GameLoad";
	case FunctionKey::CREDITS_TOGGLE:	return "ToggleCredits";
	case FunctionKey::QUIT_GAME:	return "GameQuit";
	case FunctionKey::EXIT_APP:		return "AppExit";
	default:						return "SelectButton";
	}
	return "Invalid";
}

void Hover_Logic(GameObject& button, UIButtonRegister& bReg);

// Button Component to assign function callback.
struct Button : Behaviour
{
	FunctionKey fKey;
	static UIButtonRegister* reg;
	static void SetRegister(UIButtonRegister* br) { reg = br; }

	void DrawInInspector() override
	{
		if (ImGui::BeginCombo("ButtonMode", ReturnButtonName(fKey)))
		{
			for (int i = 0; i < static_cast<int>(FunctionKey::COUNT); i++)
			{
				bool is_selected = (fKey == functionList[i]);

				if (ImGui::Selectable(ReturnButtonName(functionList[i]), is_selected))
					fKey = functionList[i];

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}
	}

	void Serialize(Json::Value& outComp) const override
	{
		outComp["functionListId"] = static_cast<int>(fKey);
	}

	void Deserialize(const Json::Value& compObj) override
	{
		if (compObj.isMember("functionListId"))
			fKey = functionList[compObj["functionListId"].asInt()];
	}

	void OnStart() override {}
	void OnUpdate() override
	{
		GameObject& owner = *_owner;
		Hover_Logic(owner, *reg);
	}
	void OnDestroy() override {}

	const std::string name() const override { return "Button"; }
};

// Preset value.
constexpr f32 defaultButtonHeight{ 100.f }, defaultButtonWidth{ 400.f }, defaultTextSize{ 40.f }, defaultStrokeWeight{ 2.f }, zeroVal{};