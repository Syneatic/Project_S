/*
Author: Zachary Yee
Co-Author: Nil
*/
#pragma once

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
	CONTROLS_MM,
	CONTROLS_GAME,
	QUIT_GAME,
	EXIT_APP,
	CONFIRMATION_MM,
	CONFIRMATION_PM,
	COUNT
};

// Enum class ID for all possible slider functions.
enum class AudioSpecifier : char
{
	GLOBAL, SFX, MUSIC, COUNT
};

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

// Slider Component to assign function callback.
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

// Declarations for ui.cpp functions.
namespace UISystem
{
	void init();
	void Update();
	void Hover_Logic(Button& button);
	void Hover_Logic(Slider& button);
	void TogglePauseMenuGame();
	GameObject& GetTimer();
	void EndScreen(bool state);
	void exit();
}