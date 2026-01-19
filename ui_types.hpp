#pragma once
#include <string>
#include "component.hpp"
#include "math.hpp"

typedef void (*CallbackF)(void);

enum UI_TYPES
{
	DISPLAY,
	BUTTON,
	TEXT
};

struct UI : Component
{
	UI_TYPES type;

	UI(UI_TYPES ui_t) : type(ui_t) {}
};

struct Display
{
	// Can be image or texture
};

struct Button : UI
{
	CallbackF callFunc;
	//UI type;

	Button(CallbackF function) : callFunc(function) {}
};

struct Text
{
	std::string str;

	Text(std::string string) : str{ string } {}
};

struct DisplayText : Display, Text
{
	DisplayText(std::string s) : Text{ s } {}
};

struct ButtonText : Button, Text
{
	ButtonText(CallbackF function, std::string string) : Button{ function }, Text{ string } {}
};

constexpr float defaultButtonHeight{ 100.f }, defaultButtonWidth{ 400.f }, defaultLabelSize{ 40.f }, defaultStrokeWeight{ 2.f }, zeroVal{};
