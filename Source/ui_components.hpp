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

// Display component to attach image or custom texture.
class Display : public Component
{
public:
	AEGfxTexture* texture{ nullptr };

	void DrawInInspector() override;
	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;

	const std::string name() const override { return "Display"; }

	Display(GameObject& go) : Component(go) {};
	void CopyFrom(Component* src) override;
	std::unique_ptr<Component> Clone(GameObject& go) override;
};

static char const* _buttonNames[]
{
	"GamePlay", "GamePause", "GameRestart", "ToggleSettings", "MusicUp", "MusicDown",
	"SfxUp", "SfxDown", "GameSave", "GameLoad", "ToggleCredits", "GameQuit", "AppExit"
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

namespace UISystem
{
	void init();
	void Hover_Logic(GameObject& button);
	void exit();
}