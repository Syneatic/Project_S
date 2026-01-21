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
	SETTINGS_TOGGLE_MAINMENU,
	SETTINGS_TOGGLE_GAME,
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

	void OnStart() override {}
	void OnUpdate() override {}
	void OnDestroy() override {}

	const std::string name() const override { return "Text"; }
};

// Button Component to assign function callback.
struct Button : Behaviour
{
	FunctionKey fKey;

	void DrawInInspector() override
	{
		ImGui::TextUnformatted("CallFunction");
	}

	void OnStart() override {}
	void OnUpdate() override {}
	void OnDestroy() override {}

	const std::string name() const override { return "Button"; }
};

// Function Declarations
void BindButtonFunctions(UIButtonRegister& bReg);
void Hover_Logic(GameObject& button, UIButtonRegister& bReg);

// Preset value.
constexpr f32 defaultButtonHeight{ 100.f }, defaultButtonWidth{ 400.f }, defaultTextSize{ 40.f }, defaultStrokeWeight{ 2.f }, zeroVal{};