#pragma once

namespace UISystem
{
	// Preset value.
	constexpr f32 defaultTextSize{ 40.f }, defaultStrokeWeight{ 2.f }, zeroVal{};
}

// Enum class ID for all possible button functions.
enum class FunctionKey
{
	PLAY_GAME,
	LOAD_GAME,
	PAUSE_GAME,
	RESTART_GAME,
	SETTINGS_MM,
	SETTINGS_GAME,
	CREDITS_TOGGLE,
	QUIT_GAME,
	EXIT_APP,
	COUNT
};

enum class AudioSpecifier : char
{
	GLOBAL, MUSIC, SFX, COUNT
};

// Display component to attach image or custom texture.
//class Display : public Component
//{
//public:
//	AEGfxTexture* texture{ nullptr };
//
//	void DrawInInspector() override;
//	void OnStart() override;
//	void OnUpdate() override;
//	void OnDestroy() override;
//
//	const std::string name() const override { return "Display"; }
//
//	Display(GameObject& go) : Component(go) {};
//	void CopyFrom(Component* src) override;
//	std::unique_ptr<Component> Clone(GameObject& go) override;
//};

// Button Component to assign function callback.
class Button : public Component
{
public:
	FunctionKey fKey{};

	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;
	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;

	const std::string name() const override { return "Button"; }

	Button(GameObject& go) : Component(go) {};
	void CopyFrom(Component* src) override;
	std::unique_ptr<Component> Clone(GameObject& go) override;
};

class Slider : public Component
{
public:
	float minX{}, maxX{};      // world-space bounds of the track
	f32 value{};           // 0.0f to 1.0f
	bool isDragging{};
	AudioSpecifier audioS{};

	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;
	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;

	const std::string name() const override { return "Slider"; }

	Slider(GameObject& go) : Component(go) {};
	void CopyFrom(Component* src) override;
	std::unique_ptr<Component> Clone(GameObject& go) override;
private:
	Transform* _trackTransform{ nullptr };  // pointer to the slider track GameObject
	f32 FindSliderVal();
public:
	Transform& TrackTransform() { return *_trackTransform; }
};

namespace UISystem
{
	void init();
	void Hover_Logic(Button& button);
	void Hover_Logic(Slider& button);
	void TogglePauseMenuGame();
	void EndScreen();
	void exit();
}