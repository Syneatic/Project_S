#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>

#include "component.hpp"
#include "renderer.hpp"
#include "math.hpp"
#include "AEEngine.h"

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

using CallbackF = void(*)(); // Identifier for void pointer function with void param.

//struct EnumHash 
//{
//	template <typename T>
//	std::size_t operator()(T t) const
//	{
//		return static_cast<std::size_t>(t);
//	}
//};

// Identifier for unordered_map with FunctionKey & Function Pointer.
using ButtonRegister = std::unordered_map<FunctionKey, CallbackF/*std::function<void()>, EnumHash*/>;

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

	// The template register to bind functions. 
	// If function is a class member the second parameter MUST be the class object itself.
	/*template<typename Func, typename... Args>
	void bindFunction(FunctionKey key, Func f, Args... args) 
	{
		std::cout << "Binding" << static_cast<int>(key) << std::endl;
		_buttonReg[key] = [f, args...]() { std::invoke(f, args...); };
	}*/

	void bindFunction(FunctionKey key, CallbackF callF)
	{
		_buttonReg[key] = callF;
		std::cout << "Binded\n";
	}

	void handleMouseClick(FunctionKey key)
	{
		std::cout << "Clicked\n";
		auto iterator = _buttonReg.find(key);

		if (iterator != _buttonReg.end() && iterator->second)
		{
			std::cout << "Function Called\n";
			iterator->second();
		}
	}

private:
	ButtonRegister _buttonReg;
	UIButtonRegister() {}
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

static char const* _buttonNames[]
{
	"GamePlay", "GamePause", "GameRestart", "ToggleSettings", "MusicUp", "MusicDown",
	"SfxUp", "SfxDown", "GameSave", "GameLoad", "ToggleCredits", "GameQuit", "AppExit"
};

namespace UISystem
{
	void BindButtonFunctions(UIButtonRegister& bReg);
	void Hover_Logic(GameObject& button, UIButtonRegister& bReg);
}

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
		//GameObject& owner = *_owner;
		UISystem::Hover_Logic(gameObject(), UIButtonRegister::Instance());
	}
	void OnDestroy() override {}

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