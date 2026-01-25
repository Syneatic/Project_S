#pragma once
#include <string>
#include "component.hpp"
#include "math.hpp"
#include "AEEngine.h"

typedef void (*CallbackF)(void);

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

// Button Component to assign function callback.
struct Button : Behaviour
{
	CallbackF callFunc{ nullptr };

	void DrawInInspector() override
	{
		ImGui::TextUnformatted("CallFunction");
	}

	void OnStart() override {}
	void OnUpdate() override {}
	void OnDestroy() override {}

	const std::string name() const override { return "Button"; }
};

// Preset value.
constexpr f32 defaultButtonHeight{ 100.f }, defaultButtonWidth{ 400.f }, defaultTextSize{ 40.f }, defaultStrokeWeight{ 2.f }, zeroVal{};
