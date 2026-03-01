using namespace ImGui;

namespace
{
	std::string FormatID(const std::string& label)
	{
		std::string id = "##";
		for (unsigned char c : label)
			if (!std::isspace(c))
				id += std::tolower(c);
		return id;
	}	
}

void FloatDrag(std::string label, float* var, float2 minmax = {0,0}, float spd = 1.0f)
{
	TextUnformatted(label.c_str());
	DragFloat(FormatID(label).c_str(), var, spd, minmax.x, minmax.y);
}

void FloatDragReset(const std::string& label, float* var, float resetVal, float speed = 1.0f, float min = 0.0f, float max = 0.0f)
{
	TextUnformatted(label.c_str());
	DragFloat(FormatID(label).c_str(), var, speed, min, max);
	SameLine();
	std::string btnLabel = "Reset##" + label;
	if (Button(btnLabel.c_str()))
		*var = resetVal;
}

void FloatSlider(const std::string& label, float* var, float min, float max)
{
	TextUnformatted(label.c_str());
	SliderFloat(FormatID(label).c_str(), var, min, max);
}

void Float2DragReset(const std::string& label, float* var, float2 resetVal, float speed = 0.05f)
{
	TextUnformatted(label.c_str());
	DragFloat2(FormatID(label).c_str(), var, speed);
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