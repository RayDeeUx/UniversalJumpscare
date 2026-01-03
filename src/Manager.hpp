#pragma once

#include "UniversalJumpscareSprite.hpp"
#include "Utils.hpp"
#include <random>

// Manager.hpp structure by acaruso
// reused with explicit permission and strong encouragement

using namespace geode::prelude;

class Manager {

protected:
	static Manager* instance;
public:

	bool calledAlready = false;

	bool hideEverywhereElse = false;
	bool hideInLevelEditorLayer = false;
	std::string visibilityInPlayLayer = "always visible";

	bool forceHideIfJumpscareStillActive = true;

	int64_t numerator = 1;
	int64_t denominator = 10000;
	double probability = static_cast<double>(numerator) / static_cast<double>(denominator);

	double probabilityFrequency = .016667;

	UniversalJumpscareSprite* unjus = nullptr;
	bool unjusIsAnimated = false;

	std::mt19937_64 rng{std::random_device{}()};
	std::uniform_real_distribution<double> dist{0.0, 1.0};

	FMOD::Sound* sound;
	FMOD::Channel* channel;
	FMOD::System* system = FMODAudioEngine::sharedEngine()->m_system;

	float jumpscareAudioVolume = .5f;
	float jumpscareFadeOutTime = .5f;
	float jumpscareFadeOutDelay = .5;

	const std::unordered_set<std::string_view> audioExtensions = {".mp3", ".wav", ".ogg", ".oga", ".flac"};
	const std::unordered_set<std::string_view> imageExtensions = {".png", ".webp", ".gif", ".jpeg", ".jpg", ".apng", ".jxl", ".qoi"};
	std::unordered_map<std::filesystem::path, std::filesystem::path> jumpscares = {};

	static Manager* get() {
		if (!instance) instance = new Manager();
		return instance;
	}

	static void loadStuff() {
		Manager::loadOtherJumpscares();
		instance->hideInLevelEditorLayer = Mod::get()->getSettingValue<bool>("hideInLevelEditorLayer");
		instance->hideEverywhereElse = Mod::get()->getSettingValue<bool>("hideEverywhereElse");
		instance->forceHideIfJumpscareStillActive = Mod::get()->getSettingValue<bool>("forceHideIfJumpscareStillActive");
		instance->visibilityInPlayLayer = geode::utils::string::toLower(Mod::get()->getSettingValue<std::string>("visibilityInPlayLayer"));
		instance->jumpscareAudioVolume = static_cast<float>(std::clamp<int>(static_cast<int>(Mod::get()->getSettingValue<int64_t>("jumpscareAudioVolume")), 0, 100)) / 100.f;
		instance->jumpscareFadeOutTime = std::clamp<float>(static_cast<float>(Mod::get()->getSettingValue<double>("jumpscareFadeOutTime")), .01f, 15.f);
		instance->jumpscareFadeOutDelay = std::clamp<float>(static_cast<float>(Mod::get()->getSettingValue<double>("jumpscareFadeOutDelay")), .0f, 15.f);
		instance->probabilityFrequency = std::clamp<double>(Mod::get()->getSettingValue<double>("probabilityFrequency"), .016667, 1000);
	}

	static void calculateProbability(const int64_t numerator, const int64_t denominator) {
		instance->numerator = std::clamp<int64_t>(numerator, 1, 1000000);
		instance->denominator = std::clamp<int64_t>(denominator, 2, 1000000);
		instance->probability = static_cast<double>(instance->numerator) / static_cast<double>(instance->denominator);
		if (instance->probability < 1.f) return;
		instance->numerator = 1;
		instance->denominator = 10000;
		instance->probability = static_cast<double>(instance->numerator) / static_cast<double>(instance->denominator);
		MDPopup::create("A friendly warning from UniversalJumpscare", "The probability has been <c-ff0000>***__forcibly reset__***</c> to <cj>1</c> out of <co>10000</c>.\n\n<c-ff0000>***__Don't try any more funny business if you know what's good for you.__***</c>", "I Understand")->show();
	}

	static bool shouldNotJumpscare() {
		return !(instance->dist(instance->rng) < instance->probability);
	}

	static bool acceptableAudioFileExtension(const std::filesystem::path& audioFile) {
	    if (!std::filesystem::exists(audioFile) || std::filesystem::is_directory(audioFile)) return false;
		return instance->audioExtensions.contains(geode::utils::string::pathToString(audioFile));
	}

	static bool acceptableImageFileExtension(const std::filesystem::path& imageFile) {
	    if (!std::filesystem::exists(imageFile) || std::filesystem::is_directory(imageFile)) return false;
		return instance->imageExtensions.contains(geode::utils::string::pathToString(imageFile));
	}

	static void loadOtherJumpscares() {
		if (!instance->jumpscares.empty()) instance->jumpscares.clear();
		Manager::loadJumpscaresFrom(Mod::get()->getSettingValue<std::filesystem::path>("jumpscaresFolder"));
		Manager::loadJumpscaresFrom(Mod::get()->getSettingValue<std::filesystem::path>("additionalJumpscaresFolder"));
	}

	static void tryFindingCorrespondingFile(const std::filesystem::path& path, const std::string& stem, std::unordered_map<std::string, std::filesystem::path>& knownFileNames, const bool trueIfImageFalseIfAudio, std::unordered_map<std::filesystem::path, std::filesystem::path>& jumpscaresMap) {
		auto correspondingJumpscareItem = knownFileNames.find(stem);
		if (correspondingJumpscareItem != knownFileNames.end()) {
			if (trueIfImageFalseIfAudio) jumpscaresMap.emplace(correspondingJumpscareItem->second, path);
			else jumpscaresMap.emplace(path, correspondingJumpscareItem->second);
			knownFileNames.erase(correspondingJumpscareItem);
		} else {
			knownFileNames.emplace(stem, path);
		}
	}

	static void checkSubDirectoryForJumpscare(const auto& file, std::unordered_map<std::filesystem::path, std::filesystem::path>& jumpscaresMap) {
		std::filesystem::path imageFilePath {};
		std::filesystem::path audioFilePath {};
		for (const auto& subFile : std::filesystem::directory_iterator(file.path())) {
			if (std::filesystem::is_directory(subFile)) continue;
			const std::string& stemAsString = string::pathToString(subFile.path().stem());
			log::info("stemAsString [subdirectory]: {}", stemAsString);
			if (!std::filesystem::exists(imageFilePath) && stemAsString == "jumpscare" && Manager::acceptableImageFileExtension(subFile.path().extension())) imageFilePath = subFile.path();
			if (!std::filesystem::exists(audioFilePath) && stemAsString == "jumpscareAudio" && Manager::acceptableAudioFileExtension(subFile.path().extension())) audioFilePath = subFile.path();
			if (std::filesystem::exists(imageFilePath) && std::filesystem::exists(audioFilePath)) break;
		}
		if (std::filesystem::exists(imageFilePath)) {
			jumpscaresMap.emplace(imageFilePath, std::filesystem::exists(audioFilePath) ? audioFilePath : std::filesystem::path{});
		}
	}

	static void loadJumpscaresFrom(const std::filesystem::path& folder) {
		if (!std::filesystem::exists(folder) || !std::filesystem::is_directory(folder)) return;

		std::unordered_map<std::string, std::filesystem::path> knownImageFiles;
		std::unordered_map<std::string, std::filesystem::path> knownAudioFiles;

		for (const auto& file : std::filesystem::directory_iterator(folder)) {
			if (!std::filesystem::exists(file)) continue;

			if (std::filesystem::is_directory(file)) {
				Manager::checkSubDirectoryForJumpscare(file, instance->jumpscares);
			} else if (std::filesystem::is_regular_file(file)) {
				const std::filesystem::path& path = file.path();
				const std::filesystem::path& extension = path.extension();
				const std::string& stemAsString = geode::utils::string::pathToString(path.stem());
				log::info("stemAsString: {}", stemAsString);

				if (Manager::acceptableImageFileExtension(extension)) {
					Manager::tryFindingCorrespondingFile(path, stemAsString, knownAudioFiles, false, instance->jumpscares);
				} else if (Manager::acceptableAudioFileExtension(extension)) {
					Manager::tryFindingCorrespondingFile(path, stemAsString, knownImageFiles, true, instance->jumpscares);
				}
			}
		}

		for (const auto& [unused, path] : knownImageFiles) if (std::filesystem::exists(path)) instance->jumpscares.emplace(path, std::filesystem::path{});
	}
};