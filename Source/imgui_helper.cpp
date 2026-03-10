#include "imgui_helper.hpp"

namespace
{
	std::string tolowerandnspc(const std::string s)
	{
		std::string out = "";
		for (unsigned char c : s)
			if (!std::isspace(c))
				out += std::tolower(c);

		return out;
	}
}


using namespace ImGui;

std::string FormatID(const std::string& label, const std::string& id)
{
	std::string back = "##";
	back += tolowerandnspc(label);
	back += "_";
	back += tolowerandnspc(label);
	return back;
}


void FloatDrag(std::string label,std::string id, float* var, float2 minmax, float spd)
{
	TextUnformatted(label.c_str());
	DragFloat(FormatID(label,id).c_str(), var, spd, minmax.x, minmax.y);
}

void FloatDragReset(const std::string& label, std::string id, float* var, float resetVal, float speed, float min, float max)
{
	TextUnformatted(label.c_str());
	DragFloat(FormatID(label,id).c_str(), var, speed, min, max);
	SameLine();
	std::string btnLabel = "Reset##" + label;
	if (Button(btnLabel.c_str()))
		*var = resetVal;
}

void FloatSlider(const std::string& label, std::string id, float* var, float min, float max)
{
	TextUnformatted(label.c_str());
	SliderFloat(FormatID(label,id).c_str(), var, min, max);
}

void Float2DragReset(const std::string& label, std::string id, float* var, float2 resetVal, float speed)
{
	TextUnformatted(label.c_str());
	DragFloat2(FormatID(label,id).c_str(), var, speed);
	SameLine();
	std::string btnLabel = "Reset##" + label;
	if (Button(btnLabel.c_str()))
	{
		var[0] = resetVal.x;
		var[1] = resetVal.y;
	}
}

// returns true if name was changed, and updates nameOut
bool NameInputText(std::string& nameOut)
{
	char buffer[256];
	strcpy_s(buffer, nameOut.c_str());
	if (InputText("##name", buffer, sizeof(buffer)))
	{
		if (strlen(buffer) > 0)
		{
			nameOut = std::string(buffer);
			return true;
		}
	}
	return false;
}


// calls onSelect(index) when a menu item is clicked
void ComponentSubMenu(const std::string& menuLabel, const std::vector<std::string>& items, std::function<void(int)> onSelect)
{
	if (BeginMenu(menuLabel.c_str()))
	{
		for (int i = 0; i < (int)items.size(); i++)
			if (MenuItem(items[i].c_str()))
				onSelect(i);
		ImGui::EndMenu();
	}
}

void SelectableList(const std::vector<std::string>& names, int& selectedIndex)
{
	for (int i = 0; i < (int)names.size(); i++)
	{
		if (ImGui::Selectable(names[i].c_str(), selectedIndex == i))
			selectedIndex = i;
	}
}

void RenderSort(std::string id,Graphics::RenderLayer& layer,float& sortOrder)
{

	const char* renderLayerNames[] = { "BACKGROUND", "DEFAULT", "UI", "GIZMOS" };
	int rlIdx = static_cast<int>(layer);
	std::string comboid = "##renderlayer_";
	comboid += id;
	if (ImGui::Combo(comboid.c_str(), &rlIdx, renderLayerNames, 4))
		layer = static_cast<Graphics::RenderLayer>(rlIdx);

	FloatDragReset("Sort Order", id, &sortOrder, 0.f);
}