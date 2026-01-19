#pragma once
#include <string>
#include "component.hpp"
#include "math.hpp"
#include "AEEngine.h"

typedef void (*CallbackF)(void);

struct UI : Behaviour
{
	void DrawInInspector() override
	{

	}
};

struct Display : UI
{
	AEGfxTexture* texture{ nullptr };

	void DrawInInspector() override
	{

	}
};

struct Text : UI
{
	std::string str{};

	void DrawInInspector() override
	{

	}
};

struct Button : UI, Display
{
	CallbackF callFunc{ nullptr };

	void DrawInInspector() override
	{

	}
};

struct DisplayText : Display, Text
{
	void DrawInInspector() override
	{

	}
};

struct ButtonText : Button, Text
{
	void DrawInInspector() override
	{

	}
};

constexpr f32 defaultButtonHeight{ 100.f }, defaultButtonWidth{ 400.f }, defaultTextSize{ 40.f }, defaultStrokeWeight{ 2.f }, zeroVal{};
