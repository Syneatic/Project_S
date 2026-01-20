#pragma once
#include <string>
#include "component.hpp"
#include "math.hpp"

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
	char textBuffer[128];

	void DrawInInspector() override
	{
		ImGui::InputText("Text", textBuffer, IM_ARRAYSIZE(textBuffer));
	}

	void OnStart() override {}
	void OnUpdate() override {}
	void OnDestroy() override {}

	const std::string name() const override { return "Text##Text"; }
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
