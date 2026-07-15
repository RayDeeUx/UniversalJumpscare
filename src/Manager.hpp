#pragma once

#include "UniversalJumpscareSprite.hpp"
#include <random>

// Manager.hpp structure by acaruso
// reused with explicit permission and strong encouragement

using namespace geode::prelude;

typedef std::unordered_map<std::string, std::filesystem::path> StemToPath;
typedef std::unordered_map<std::filesystem::path, std::filesystem::path> ImageToOptionalAudio;

enum class JumpscareType {
	RandomTimer,
	GameplayClick,
	GameplayDeath
};

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

	int64_t numeratorClickBased = 1;
	int64_t denominatorClickBased = 10000;
	double probabilityClickBased = static_cast<double>(numeratorClickBased) / static_cast<double>(denominatorClickBased);

	int64_t numeratorDeathBased = 1;
	int64_t denominatorDeathBased = 10000;
	double probabilityDeathBased = static_cast<double>(numeratorDeathBased) / static_cast<double>(denominatorDeathBased);

	double probabilityFrequency = (1. / 60.);

	UniversalJumpscareSprite* unjus = nullptr;
	bool unjusIsAnimated = false;

	std::mt19937_64 rng{std::random_device{}()};
	std::uniform_real_distribution<double> dist{0.0, 1.0};

	std::mt19937_64 rngMore{std::random_device{}()};

	FMOD::Sound* sound;
	FMOD::Channel* channel;
	FMOD::System* system = FMODAudioEngine::sharedEngine()->m_system;

	float jumpscareAudioVolume = .5f;
	float jumpscareFadeOutTime = .5f;
	float jumpscareFadeOutDelay = .5;

	const std::unordered_set<std::string_view> audioExtensions = {".mp3", ".wav", ".ogg", ".oga", ".flac"};
	const std::unordered_set<std::string_view> imageExtensions = {".png", ".webp", ".gif", ".jpeg", ".jpg", ".apng", ".jxl", ".qoi"};
	ImageToOptionalAudio jumpscares = {};
	bool randomizeJumpscares = false;

	std::filesystem::path currentImage {};
	std::filesystem::path currentAudio {};

	double timePassed = 0;

	bool jumpscareOnRandomTimer = false;
	bool jumpscareOnClick = false;
	bool jumpscareOnDeath = false;

	static Manager* get() {
		if (!instance) instance = new Manager();
		return instance;
	}

	static void loadStuff() {
		Manager::loadOtherJumpscares();
		instance->hideInLevelEditorLayer = Mod::get()->getSettingValue<bool>("hideInLevelEditorLayer");
		instance->hideEverywhereElse = Mod::get()->getSettingValue<bool>("hideEverywhereElse");
		instance->forceHideIfJumpscareStillActive = Mod::get()->getSettingValue<bool>("forceHideIfJumpscareStillActive");
		instance->randomizeJumpscares = Mod::get()->getSettingValue<bool>("randomizeJumpscares");
		instance->visibilityInPlayLayer = geode::utils::string::toLower(Mod::get()->getSettingValue<std::string>("visibilityInPlayLayer"));
		instance->jumpscareAudioVolume = static_cast<float>(std::clamp<int>(static_cast<int>(Mod::get()->getSettingValue<int64_t>("jumpscareAudioVolume")), 0, 100)) / 100.f;
		instance->jumpscareFadeOutTime = std::clamp<float>(static_cast<float>(Mod::get()->getSettingValue<double>("jumpscareFadeOutTime")), .01f, 15.f);
		instance->jumpscareFadeOutDelay = std::clamp<float>(static_cast<float>(Mod::get()->getSettingValue<double>("jumpscareFadeOutDelay")), .0f, 15.f);
		instance->probabilityFrequency = std::clamp<double>(Mod::get()->getSettingValue<double>("probabilityFrequency"), (1. / 60.), 1000);
		instance->jumpscareOnRandomTimer = Mod::get()->getSettingValue<bool>("jumpscareOnRandomTimer");
		instance->jumpscareOnClick = Mod::get()->getSettingValue<bool>("jumpscareOnClick");
		instance->jumpscareOnDeath = Mod::get()->getSettingValue<bool>("jumpscareOnDeath");
		Manager::calculateProbability(Mod::get()->getSettingValue<int64_t>("probabilityNumerator"), Mod::get()->getSettingValue<int64_t>("probabilityDenominator"), JumpscareType::RandomTimer);
		Manager::calculateProbability(Mod::get()->getSettingValue<int64_t>("probabilityNumeratorClickBased"), Mod::get()->getSettingValue<int64_t>("probabilityDenominatorClickBased"), JumpscareType::GameplayClick);
		Manager::calculateProbability(Mod::get()->getSettingValue<int64_t>("probabilityNumeratorDeathBased"), Mod::get()->getSettingValue<int64_t>("probabilityDenominatorDeathBased"), JumpscareType::GameplayDeath);
	}

	static void calculateProbability(const int64_t numerator, const int64_t denominator, JumpscareType jumpscare) {
		const int64_t numeratorClamped = std::clamp<int64_t>(numerator, 1, 1000000);
		const int64_t denominatorClamped = std::clamp<int64_t>(denominator, 2, 1000000);

		if (jumpscare == JumpscareType::GameplayClick) {
			if (instance->numeratorClickBased == numeratorClamped && instance->denominatorClickBased == denominatorClamped) return;
			instance->numeratorClickBased = numeratorClamped;
			instance->denominatorClickBased = denominatorClamped;
			instance->probabilityClickBased = static_cast<double>(instance->numeratorClickBased) / static_cast<double>(instance->denominatorClickBased);
			if (instance->probabilityClickBased < 1.f && instance->probabilityClickBased > 0.f) return;
			instance->numeratorClickBased = 1;
			instance->denominatorClickBased = 10000;
			instance->probabilityClickBased = static_cast<double>(instance->numeratorClickBased) / static_cast<double>(instance->denominatorClickBased);
			return MDPopup::create("A \"friendly\" \"warning\" from UniversalJumpscare", "The probability for <cl>click-based jumpscares</c> has been <c-ff0000>***__forcibly reset__***</c> to <cj>1</c> out of <co>10000</c>.\n\n# <c-ff0000>***__Don't try any more funny business if you know what's good for you.__***</c>", "I Understand")->show();
		}

		if (jumpscare == JumpscareType::GameplayDeath) {
			if (instance->numeratorDeathBased == numeratorClamped && instance->denominatorDeathBased == denominatorClamped) return;
			instance->numeratorDeathBased = numeratorClamped;
			instance->denominatorDeathBased = denominatorClamped;
			instance->probabilityDeathBased = static_cast<double>(instance->numeratorDeathBased) / static_cast<double>(instance->denominatorDeathBased);
			if (instance->probabilityDeathBased < 1.f && instance->probabilityDeathBased > 0.f) return;
			instance->numeratorDeathBased = 1;
			instance->denominatorDeathBased = 10000;
			instance->probabilityDeathBased = static_cast<double>(instance->numeratorDeathBased) / static_cast<double>(instance->denominatorDeathBased);
			return MDPopup::create("A \"friendly\" \"warning\" from UniversalJumpscare", "The probability for <cl>death-based jumpscares</c> has been <c-ff0000>***__forcibly reset__***</c> to <cj>1</c> out of <co>10000</c>.\n\n# <c-ff0000>***__Don't try any more funny business if you know what's good for you.__***</c>", "I Understand")->show();
		}

		if (instance->numerator == numeratorClamped && instance->denominator == denominatorClamped) return;
		instance->numerator = numeratorClamped;
		instance->denominator = denominatorClamped;
		instance->probability = static_cast<double>(instance->numerator) / static_cast<double>(instance->denominator);
		if (instance->probability < 1.f && instance->probability > 0.f) return;
		instance->numerator = 1;
		instance->denominator = 10000;
		instance->probability = static_cast<double>(instance->numerator) / static_cast<double>(instance->denominator);
		MDPopup::create("A \"friendly\" \"warning\" from UniversalJumpscare", "The probability for <cl>random timer jumpscares</c> has been <c-ff0000>***__forcibly reset__***</c> to <cj>1</c> out of <co>10000</c>.\n\n# <c-ff0000>***__Don't try any more funny business if you know what's good for you.__***</c>", "I Understand")->show();
	}

	static bool shouldNotJumpscare(JumpscareType jumpscare) {
		if (jumpscare == JumpscareType::GameplayClick) return !instance->jumpscareOnClick || !(instance->dist(instance->rng) < instance->probabilityClickBased);
		if (jumpscare == JumpscareType::GameplayDeath) return !instance->jumpscareOnDeath || !(instance->dist(instance->rng) < instance->probabilityDeathBased);
		return !instance->jumpscareOnRandomTimer || !(instance->dist(instance->rng) < instance->probability);
	}

	static bool acceptableAudioFileExtension(const std::filesystem::path& audioFile) {
		std::error_code eb, ec;
		if (!std::filesystem::exists(audioFile, eb) || std::filesystem::is_directory(audioFile, ec)) return false;
		return instance->audioExtensions.contains(geode::utils::string::pathToString(audioFile.extension()));
	}

	static bool acceptableImageFileExtension(const std::filesystem::path& imageFile) {
		std::error_code eb, ec;
		if (!std::filesystem::exists(imageFile, eb) || std::filesystem::is_directory(imageFile, ec)) return false;
		return instance->imageExtensions.contains(geode::utils::string::pathToString(imageFile.extension()));
	}

	static void loadOtherJumpscares() {
		if (!instance->jumpscares.empty()) instance->jumpscares.clear();
		Manager::loadJumpscaresFrom(Mod::get()->getSettingValue<std::filesystem::path>("jumpscaresFolder"), instance->jumpscares);
		Manager::loadJumpscaresFrom(Mod::get()->getSettingValue<std::filesystem::path>("additionalJumpscaresFolder"), instance->jumpscares);
		for (const auto&[imageFile, audioFile] : instance->jumpscares) {
			log::info("imageFile: {}", geode::utils::string::pathToString(imageFile));
			log::info("audioFile: {}", geode::utils::string::pathToString(audioFile));
		}
	}

	static void tryFindingCorrespondingFile(const std::filesystem::path& path, const std::string& stem, StemToPath& imageStemFilePairs, StemToPath& audioStemFilePairs, ImageToOptionalAudio& jumpscaresMap) {
		std::error_code ea, eb;
		if (Manager::acceptableImageFileExtension(path)) {
			auto audioStemFilePair = audioStemFilePairs.find(stem);
			if (audioStemFilePair != audioStemFilePairs.end() && std::filesystem::exists(audioStemFilePair->second, ea)) {
				jumpscaresMap.emplace(path, audioStemFilePair->second);
				audioStemFilePairs.erase(audioStemFilePair);
			} else {
				imageStemFilePairs.emplace(stem, path);
			}
		} else if (Manager::acceptableAudioFileExtension(path)) {
			auto imageStemFilePair = imageStemFilePairs.find(stem);
			if (imageStemFilePair != imageStemFilePairs.end() && std::filesystem::exists(imageStemFilePair->second, eb)) {
				jumpscaresMap.emplace(imageStemFilePair->second, path);
				imageStemFilePairs.erase(imageStemFilePair);
			} else {
				audioStemFilePairs.emplace(stem, path);
			}
		}
	}

	static void checkSubDirectoryForJumpscare(const std::filesystem::directory_entry& dirEntry, ImageToOptionalAudio& jumpscaresMap) {
		std::error_code ea, eb, ec, ed;
		if (!std::filesystem::is_directory(dirEntry, ea)) return;
		std::filesystem::path imageFile {};
		std::filesystem::path audioFile {};
		bool imageFound = false;
		bool audioFound = false;

		for (const auto& subFile : std::filesystem::directory_iterator(dirEntry.path())) {
			if (!std::filesystem::is_regular_file(subFile, eb)) continue;
			const std::filesystem::path& subPath = subFile.path();
			const std::string& stem = geode::utils::string::pathToString(subPath.stem());
			if (!imageFound && stem == "jumpscare" && Manager::acceptableImageFileExtension(subPath)) {
				imageFile = subPath;
				imageFound = true;
			}
			if (!audioFound && stem == "jumpscareAudio" && Manager::acceptableAudioFileExtension(subPath)) {
				audioFile = subPath;
				audioFound = true;
			}
			if (imageFound && audioFound) break;
		}

		if (imageFound && std::filesystem::exists(imageFile, ec)) {
			jumpscaresMap.emplace(imageFile, (audioFound && std::filesystem::exists(audioFile, ed)) ? audioFile : std::filesystem::path{});
		}
	}

	static void loadJumpscaresFrom(const std::filesystem::path& folder, ImageToOptionalAudio& jumpscaresMap) {
		std::error_code ea, eb, ec, ed, ee, ef, eg, eh;
		if (!std::filesystem::exists(folder, ea) || !std::filesystem::is_directory(folder, eb)) return;

		StemToPath knownImageFiles;
		StemToPath knownAudioFiles;

		for (const auto& entry : std::filesystem::directory_iterator(folder)) {
			if (!std::filesystem::exists(entry, ec)) continue;

			if (std::filesystem::is_directory(entry, ed)) {
				Manager::checkSubDirectoryForJumpscare(entry, jumpscaresMap);
				continue;
			}

			if (!std::filesystem::is_regular_file(entry, ee)) continue;

			const auto path = entry.path();
			const auto stem = geode::utils::string::pathToString(path.stem());

			Manager::tryFindingCorrespondingFile(path, stem, knownImageFiles, knownAudioFiles, jumpscaresMap);
		}

		std::vector<std::string> toRemove = {};

		for (const auto& [stem, imageFile] : knownImageFiles) {
			auto audioStemFilePair = knownAudioFiles.find(stem);
			if (audioStemFilePair != knownAudioFiles.end() && std::filesystem::exists(imageFile, ef) && std::filesystem::exists(audioStemFilePair->second, eg)) {
				jumpscaresMap.emplace(imageFile, audioStemFilePair->second);
				knownAudioFiles.erase(audioStemFilePair);
				toRemove.push_back(stem);
			}
		}

		for (const auto& stem : toRemove) knownImageFiles.erase(stem);

		for (const auto& [stem, imagePath] : knownImageFiles) {
			if (stem == "background" && geode::utils::string::pathToString(imagePath.extension()) == ".png" && geode::utils::string::endsWith(geode::utils::string::pathToString(imagePath.parent_path()), "weebify.jumpscare")) {
				continue;
			}
			if (std::filesystem::exists(imagePath, eh)) jumpscaresMap.emplace(imagePath, std::filesystem::path{});
		}
	}

	static std::unordered_map<std::filesystem::path, std::filesystem::path>::const_iterator pickRandomJumpscare(const ImageToOptionalAudio& jumpscares) {
		if (jumpscares.empty()) return jumpscares.end();

		std::uniform_int_distribution<std::size_t> dist(0, jumpscares.size() - 1);

		const std::size_t selected = dist(instance->rngMore);
		std::unordered_map<std::filesystem::path, std::filesystem::path>::const_iterator iterator = jumpscares.begin();

		for (std::size_t i = 0; i < selected; ++i) ++iterator;

		return iterator;
	}
};