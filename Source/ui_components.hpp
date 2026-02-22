#pragma once

#include <unordered_map>

#include "AEEngine.h"

#include "base_components.hpp"

namespace UISystem
{
	// Preset value.
	constexpr f32 defaultTextSize{ 40.f }, defaultStrokeWeight{ 2.f }, zeroVal{};
}

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

//using CallbackF = void(*)(); // Identifier for void pointer function with void param.
// Identifier for unordered_map with FunctionKey & Function Pointer.
//using ButtonRegister = std::unordered_map<FunctionKey, CallbackF/*std::function<void()>, EnumHash*/>;

// Display component to attach image or custom texture.
struct Display : Behaviour
{
	AEGfxTexture* texture{ nullptr };

	void DrawInInspector() override;
	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;

	const std::string name() const override { return "Display"; }
};

static char const* _buttonNames[]
{
	"GamePlay", "GamePause", "GameRestart", "ToggleSettings", "MusicUp", "MusicDown",
	"SfxUp", "SfxDown", "GameSave", "GameLoad", "ToggleCredits", "GameQuit", "AppExit"
};

namespace UISystem
{
	void init();
	void Hover_Logic(GameObject& button);
}

// Button Component to assign function callback.
struct Button : Behaviour
{
	FunctionKey fKey{};

	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;
	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;

	const std::string name() const override { return "Button"; }
};

// Text component to assign text on screen.
//struct Text : Behaviour
//{
//	f32 fontSize{UISystem::defaultTextSize};
//	std::string str;
//	//static char cStr[128];
//
//	void DrawInInspector() override
//	{
//		char cStr[128]; strcpy_s(cStr, str.c_str());
//		ImGui::InputText("Text##Text", cStr, IM_ARRAYSIZE(cStr));
//		str = cStr;
//	}
//
//	void Serialize(Json::Value& outComp) const override
//	{
//		outComp["string"] = str;
//	}
//
//	void Deserialize(const Json::Value& compObj) override
//	{
//		if (compObj.isMember("string"))
//			str = compObj["string"].asString();
//	}
//
//	void OnStart() override {}
//	void OnUpdate() override 
//	{
//		//char cStr[128]; strcpy_s(cStr, str.c_str());
//		//RenderSystem::DrawMyText(cStr, gameObject().GetComponent<Transform>()->position, fontSize);
//	}
//	void OnDestroy() override {}
//
//	const std::string name() const override { return "Text"; }
//};