#include "eventhandler.hpp"
#include "ui_components.hpp"
#include "gameobject.hpp"

//void Display::DrawInInspector()
//{
//	ImGui::TextUnformatted("Texture");
//}
//
//void Display::OnStart()  {}
//void Display::OnUpdate() {}
//void Display::OnDestroy(){}
//
//void Display::CopyFrom(Component* src)
//{
//	auto s = dynamic_cast<Display*>(src);
//	if (!s) return;
//}
//
//std::unique_ptr<Component> Display::Clone(GameObject& go)
//{
//	auto n = std::make_unique<Display>(go);
//	n.get()->CopyFrom(this);
//	return n;
//}

static char const* _buttonNames[]
{
	"GamePlay", "GameLoad", "GamePause", 
	"GameRestart", "SettingsMM", "SettingsGame", 
	"ToggleCredits", "GameQuit", "AppExit"
};

void Button::DrawInInspector()
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

void Button::Serialize(Json::Value& outComp) const
{
	outComp["buttonFunctionId"] = static_cast<int>(fKey);
}

void Button::Deserialize(const Json::Value& compObj)
{
	if (compObj.isMember("buttonFunctionId"))
		fKey = static_cast<FunctionKey>(compObj["buttonFunctionId"].asInt());
}

void Button::OnStart() {}
void Button::OnUpdate()
{
	UISystem::Hover_Logic(*this);
}
void Button::OnDestroy() {}

void Button::CopyFrom(Component* src)
{
	auto s = dynamic_cast<Button*>(src);
	if (!s) return;
	fKey = s->fKey;
}

std::unique_ptr<Component> Button::Clone(GameObject& go)
{
	auto n = std::make_unique<Button>(go);
	n.get()->CopyFrom(this);
	return n;
}

static char const* _audioSpecifier[]
{
	"Global", "SFX", "Music"
};

void Slider::DrawInInspector()
{
	if (ImGui::BeginCombo("AudioSpecifier", "Global"))
	{
		for (int i = 0; i < static_cast<int>(AudioSpecifier::COUNT); i++)
		{
			bool is_selected = (i == static_cast<int>(audioS));

			if (ImGui::Selectable(_audioSpecifier[i], is_selected))
				audioS = static_cast<AudioSpecifier>(i);

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}
}

void Slider::Serialize(Json::Value& outComp) const
{
	outComp["value"] = value;
	outComp["audioS"] = static_cast<int>(audioS);
}

void Slider::Deserialize(const Json::Value& compObj)
{
	if (compObj.isMember("value"))
		value = compObj["value"].asFloat();

	if (compObj.isMember("audioS"))
		audioS = static_cast<AudioSpecifier>(compObj["audioS"].asInt());
}

void Slider::OnStart() 
{
	// Get the track gameobject (should be a child/parent).
	_trackTransform = &_owner.parent()->transform();
	float halfTrack = _trackTransform->scale.x / 2.f;
	minX = (_trackTransform->position.x - halfTrack);
	maxX = (_trackTransform->position.x + halfTrack);
	isDragging = false;
}

void Slider::OnUpdate()
{
	//_owner.UpdateWorldTransform(&_owner.parent()->worldTransform());
	Debug::Log(worldTransform().position);
	Debug::Log(transform().position);
	UISystem::Hover_Logic(*this);
}
void Slider::OnDestroy() {}

void Slider::CopyFrom(Component* src)
{
	auto s = dynamic_cast<Slider*>(src);
	if (!s) return;
}

std::unique_ptr<Component> Slider::Clone(GameObject& go)
{
	auto n = std::make_unique<Slider>(go);
	n.get()->CopyFrom(this);
	return n;
}