#include "audio_components.hpp"
#include "audio.hpp"

namespace
{
	std::wstring OpenFileWav()
	{
		namespace fs = std::filesystem;
		std::wstring targetDir = L"../../Assets/";

		try {
			if (fs::exists(targetDir)) {
				targetDir = fs::absolute(targetDir).wstring();
			}
		}
		catch (...) {}

		HRESULT hrInit = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		bool didCoInit = SUCCEEDED(hrInit) || hrInit == RPC_E_CHANGED_MODE;

		IFileOpenDialog* dialog = nullptr;
		if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr,
			CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog))))
		{
			if (didCoInit && hrInit != RPC_E_CHANGED_MODE)
				CoUninitialize();
			return L"";
		}

		dialog->SetTitle(L"Select Audio Clip");

		COMDLG_FILTERSPEC filters[] = {
			{ L"WAV files (*.wav)", L"*.wav" },
			{ L"MP3 files (*.mp3)", L"*.mp3" },
			{ L"All files (*.*)",       L"*.*" }

		};
		dialog->SetFileTypes((UINT)std::size(filters), filters);
		dialog->SetFileTypeIndex(1);

		IShellItem* startFolder = nullptr;
		if (SUCCEEDED(SHCreateItemFromParsingName(targetDir.c_str(), nullptr, IID_PPV_ARGS(&startFolder))))
		{
			dialog->SetFolder(startFolder);
			dialog->SetDefaultFolder(startFolder);
			startFolder->Release();
		}

		std::wstring result;

		if (SUCCEEDED(dialog->Show(nullptr)))
		{
			IShellItem* item = nullptr;
			if (SUCCEEDED(dialog->GetResult(&item)))
			{
				PWSTR path = nullptr;
				if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)))
				{
					result = path;
					CoTaskMemFree(path);
				}
				item->Release();
			}
		}

		dialog->Release();
		if (didCoInit && hrInit != RPC_E_CHANGED_MODE)
			CoUninitialize();

		return result;
	}
}

void AudioEmitter::SetVolume(f32 vol)
{
	volume = std::clamp(vol, 0.0f, 1.0f);
}

void AudioEmitter::SetPitch(f32 p)
{
	pitch = std::clamp(p, 0.5f, 2.0f);
}

void AudioEmitter::SetLoop(bool l)
{
	loop = l;
}

void AudioEmitter::OnStart()
{
	Audio::RegisterEmitter(this);
	auto& sound = *soundPtr.get();
	sound.setVolume(volume);
	sound.setPitch(pitch);
	sound.setLooping(loop);
	sound.setSpatializationEnabled(spatialize);
	sound.setRelativeToListener(relativeToListener); //follows the listener
	sound.setMinDistance(10.f);
	sound.setAttenuation(0.2f);

	
}

void AudioEmitter::Play()
{
	auto& sound = *soundPtr.get();
	sound.setVolume(volume);
	sound.setPitch(pitch);
	sound.setLooping(loop);
	sound.setSpatializationEnabled(spatialize);
	sound.setRelativeToListener(relativeToListener); //follows the listener
	sound.setMinDistance(10.f);
	sound.setAttenuation(0.2f);
	sound.play();
}

void AudioEmitter::Play(sf::Sound clip)
{
	clip.setVolume(volume);
	clip.setPitch(pitch);
	clip.setLooping(loop);
	clip.setSpatializationEnabled(spatialize);
	clip.setRelativeToListener(relativeToListener); //follows the listener
	clip.setMinDistance(10.f);
	clip.setAttenuation(0.2f);
	clip.play();
}

void AudioEmitter::DrawInInspector()
{
	ImGui::TextUnformatted("Audio Clip");

	ImGui::BeginDisabled();
	ImGui::TextUnformatted( fileName.empty() ? "NO FILE SELECTED" : fileName.c_str());
	ImGui::EndDisabled();

	ImGui::SameLine();

	if (ImGui::Button("Select"))
	{
		std::wstring path = OpenFileWav(); // file dialog
		fileName = std::filesystem::path(path).filename().string();
	}

	ImGui::Separator();

	ImGui::TextUnformatted("Volume");
	ImGui::SliderFloat("##audio_volume", &volume, 0.f, 1.f);;
	ImGui::TextUnformatted("Pitch");
	ImGui::SliderFloat("##audio_pitch", &pitch, 0.5f, 2.f);;

	ImGui::Checkbox("Loop", &loop);
	ImGui::Checkbox("Spatialize", &spatialize);
	ImGui::Checkbox("Relative", &relativeToListener);
}

void AudioEmitter::Serialize(Json::Value& outComp) const 
{
	outComp["fileName"] = fileName;
	outComp["volume"] = volume;
	outComp["pitch"] = pitch;
	outComp["loop"] = loop;
	outComp["spatialize"] = spatialize;
	outComp["relative"] = relativeToListener;
}

void AudioEmitter::Deserialize(const Json::Value& compObj)
{

	volume = compObj["volume"].asFloat();

	if (compObj.isMember("fileName") && compObj["fileName"].isString())
		fileName = compObj["fileName"].asString();

	if (compObj.isMember("position") && compObj["position"].isNumeric()) 
		volume = compObj["position"].asFloat();

	if (compObj.isMember("pitch") && compObj["pitch"].isNumeric())    
		pitch = compObj["pitch"].asFloat();

	if (compObj.isMember("loop") && compObj["loop"].isBool())
		loop = compObj["loop"].asBool();

	if (compObj.isMember("spatialize") && compObj["spatialize"].isBool())
		spatialize = compObj["spatialize"].asBool();

	if (compObj.isMember("relative") && compObj["relative"].isBool())
		relativeToListener = compObj["relative"].asBool();
}

void AudioEmitter::CopyFrom(Component* src)
{
	auto s = dynamic_cast<AudioEmitter*>(src);
	if (!s) return;

	volume = s->volume;
	pitch = s->pitch;
	loop = s->loop;
	spatialize = s->spatialize;
	relativeToListener = s->relativeToListener;
	fileName = std::string(s->fileName);
}

std::unique_ptr<Component> AudioEmitter::Clone(GameObject& go)
{
	auto n = std::make_unique<AudioEmitter>(go);
	n.get()->CopyFrom(this);
	return n;
}







void AudioListener::OnStart()
{
	sf::Listener::setUpVector(sf::Vector3f(0.f, 1.f, 0.f));
	sf::Listener::setDirection(sf::Vector3f(0.f, 0.f, -1.f));
}

void AudioListener::OnUpdate()
{
	sf::Vector3f pos(_transform.position.x, _transform.position.y, 10.f);
	sf::Listener::setPosition(pos);
}

void AudioListener::DrawInInspector()
{

}

void AudioListener::Serialize(Json::Value& /*outComp*/) const
{

}

void AudioListener::Deserialize(const Json::Value& /*compObj*/)
{

}

void AudioListener::CopyFrom(Component* /*src*/)
{
	
}

std::unique_ptr<Component> AudioListener::Clone(GameObject& go)
{
	auto n = std::make_unique<AudioListener>(go);
	n.get()->CopyFrom(this);
	return n;
}


void MusicPlayer::OnStart() 
{
	Audio::RegisterMusic(this);
	Audio::PlayMusic();
}

void MusicPlayer::SetVolume(f32 vol)
{
	volume = std::clamp(vol, 0.0f, 1.0f);
}

void MusicPlayer::SetLoop(bool l)
{
	loop = l;
}

void MusicPlayer::Play()
{
	Audio::PlayMusic();
}

void MusicPlayer::Stop()
{
	Audio::StopMusic();
}

void MusicPlayer::DrawInInspector() 
{
	ImGui::TextUnformatted("Music Clip");

	ImGui::BeginDisabled();
	ImGui::TextUnformatted(fileName.empty() ? "NO FILE SELECTED" : fileName.c_str());
	ImGui::EndDisabled();

	ImGui::SameLine();

	if (ImGui::Button("Select"))
	{
		std::wstring path = OpenFileWav(); // file dialog
		fileName = std::filesystem::path(path).filename().string();
	}

	ImGui::Separator();

	ImGui::TextUnformatted("Volume");
	ImGui::SliderFloat("##music_volume", &volume, 0.f, 1.f);;

	ImGui::Checkbox("Loop", &loop);
}

void MusicPlayer::Serialize(Json::Value& outComp) const 
{
	outComp["fileName"] = fileName;
	outComp["volume"] = volume;
	outComp["loop"] = loop;
}

void MusicPlayer::Deserialize(const Json::Value& compObj) 
{

	volume = compObj["volume"].asFloat();

	if (compObj.isMember("fileName") && compObj["fileName"].isString())
		fileName = compObj["fileName"].asString();

	if (compObj.isMember("loop") && compObj["loop"].isBool())
		loop = compObj["loop"].asBool();
}

void MusicPlayer::CopyFrom(Component* src)
{
	auto s = dynamic_cast<MusicPlayer*>(src);
	if (!s) return;

	volume = s->volume;
	loop = s->loop;
	fileName = std::string(s->fileName);
}

std::unique_ptr<Component> MusicPlayer::Clone(GameObject& go)
{
	auto n = std::make_unique<MusicPlayer>(go);
	n.get()->CopyFrom(this);
	return n;
}