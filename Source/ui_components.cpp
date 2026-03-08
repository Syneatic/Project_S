#include "eventhandler.hpp"
#include "ui_components.hpp"

void Display::DrawInInspector()
{
	ImGui::TextUnformatted("Texture");
}

void Display::OnStart()  {}
void Display::OnUpdate() {}
void Display::OnDestroy(){}

void Display::CopyFrom(Component* src)
{
	auto s = dynamic_cast<Display*>(src);
	if (!s) return;
}

std::unique_ptr<Component> Display::Clone(GameObject& go)
{
	auto n = std::make_unique<Display>(go);
	n.get()->CopyFrom(this);
	return n;
}


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
	UISystem::Hover_Logic(gameObject());
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
