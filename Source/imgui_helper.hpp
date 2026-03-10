#pragma once

std::string FormatID(const std::string& label, const std::string& id);
//some helper functions for drawing elements of editor ui
void FloatDrag(std::string label, std::string id, float* var, float2 minmax = { 0,0 }, float spd = 1.0f);
void FloatDragReset(const std::string& label, std::string id , float* var, float resetVal, float speed = 1.0f, float min = 0.0f, float max = 0.0f);
void FloatSlider(const std::string& label, std::string id , float* var, float min, float max);
void Float2DragReset(const std::string& label, std::string id , float* var, float2 resetVal, float speed = 0.05f);

bool NameInputText(std::string& nameOut);

void ComponentSubMenu(const std::string& menuLabel, const std::vector<std::string>& items, std::function<void(int)> onSelect);
void SelectableList(const std::vector<std::string>& names, int& selectedIndex);

void RenderSort(std::string id, Graphics::RenderLayer& layer, float& sortOrder);

template<typename T>
bool EnumCombo(const std::string& label, std::string id, T& currentValue, const std::initializer_list<std::string>& names)
{
	ImGui::TextUnformatted(label.c_str());
	int current = (int)currentValue;
	if (ImGui::Combo(FormatID(label,id).c_str(), &current,
		[](void* data, int idx, const char** out) -> bool {
			*out = ((std::vector<std::string>*)data)->at(idx).c_str();
			return true;
		}, (void*)&names, (int)names.size()))
	{
		currentValue = (T)current;
		return true;
	}
	return false;
}

