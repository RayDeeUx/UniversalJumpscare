#include "UniversalJumpscareSprite.hpp"
#include "Settings.hpp"
#include "Manager.hpp"
#include "Utils.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
	(void) Mod::get()->registerCustomSettingType("jumpscaresfolder", &MyButtonSettingV3::parse);
	(void) Mod::get()->registerCustomSettingType("additionaljumpscaresfolder", &MyButtonSettingV3::parse);
	Manager* manager = Manager::get();
	Manager::loadStuff(); // avoid segfaults
	Mod::get()->setLoggingEnabled(Mod::get()->getSettingValue<bool>("logging"));

	if (!std::filesystem::exists(Mod::get()->getConfigDir())) std::filesystem::create_directory(Mod::get()->getConfigDir());

	const std::filesystem::path& jumpscareAudio = Mod::get()->getSettingValue<std::filesystem::path>("jumpscareAudio");
	if (std::filesystem::exists(jumpscareAudio) && Manager::acceptableAudioFileExtension(jumpscareAudio)) {
		manager->currentAudio = jumpscareAudio;
		manager->system->createSound(geode::utils::string::pathToString(jumpscareAudio).c_str(), FMOD_DEFAULT, nullptr, &manager->sound);
	}
	manager->channel->setVolume(static_cast<float>(std::clamp<int>(static_cast<int>(Mod::get()->getSettingValue<int64_t>("jumpscareAudioVolume")), 0, 100)) / 100.f);

	listenForSettingChanges<bool>("enabled", [](const bool isEnabled) {
		const UniversalJumpscareSprite* unjus = Utils::getUNJUS();
		if (!isEnabled && unjus) return Utils::removeUNJUS();
		if (isEnabled && !unjus) return Utils::addUNJUS(Manager::get()->currentImage);
	});
	listenForSettingChanges<std::filesystem::path>("jumpscareImage", [](const std::filesystem::path& path) {
		if (Manager::get()->randomizeJumpscares) return;
		Utils::removeUNJUS();
		if (!Manager::acceptableImageFileExtension(path)) return;
		Manager::get()->currentImage = path;
		Utils::addUNJUS(path);
	});
	listenForSettingChanges<int64_t>("probabilityNumerator", [](const int64_t newNumerator) {
		Manager::calculateProbability(newNumerator, Mod::get()->getSettingValue<int64_t>("probabilityDenominator"));
	});
	listenForSettingChanges<int64_t>("probabilityDenominator", [](const int64_t newDenominator) {
		Manager::calculateProbability(Mod::get()->getSettingValue<int64_t>("probabilityNumerator"), newDenominator);
	});
	listenForSettingChanges<double>("probabilityFrequency", [](const double probabilityFrequency) {
		Manager::get()->probabilityFrequency = std::clamp<double>(probabilityFrequency, .016667, 1000);
	});
	listenForSettingChanges<std::string>("jumpscareImageFillMode", [](const std::string&) {
		if (UniversalJumpscareSprite* unjus = Utils::getUNJUS(); unjus) Utils::setUNJUSScale(unjus);
	});
	listenForSettingChanges<bool>("forceHideIfJumpscareStillActive", [](const bool forceHideIfJumpscareStillActiveNew) {
		Manager::get()->forceHideIfJumpscareStillActive = forceHideIfJumpscareStillActiveNew;
	});
	listenForSettingChanges<bool>("hideInLevelEditorLayer", [](const bool hideInLevelEditorLayerNew) {
		Manager::get()->hideInLevelEditorLayer = hideInLevelEditorLayerNew;
	});
	listenForSettingChanges<bool>("hideEverywhereElse", [](const bool hideEverywhereElseNew) {
		Manager::get()->hideEverywhereElse = hideEverywhereElseNew;
	});
	listenForSettingChanges<std::string>("visibilityInPlayLayer", [](const std::string& visibilityInPlayLayerNew) {
		Manager::get()->visibilityInPlayLayer = geode::utils::string::toLower(visibilityInPlayLayerNew);
	});
	listenForSettingChanges<std::filesystem::path>("jumpscareAudio", [](const std::filesystem::path& jumpscareAudioNew) {
		Manager* manager = Manager::get();
		manager->channel->stop();
		FMOD::Sound* originalSound = manager->sound;

		manager->currentAudio = std::filesystem::path{};

		if (std::filesystem::exists(jumpscareAudioNew) && Manager::acceptableAudioFileExtension(jumpscareAudioNew)) {
			manager->currentAudio = jumpscareAudioNew;
			manager->system->createSound(geode::utils::string::pathToString(jumpscareAudioNew).c_str(), FMOD_DEFAULT, nullptr, &manager->sound);
		}
		originalSound->release();
	});
	listenForSettingChanges<int64_t>("jumpscareAudioVolume", [](const int64_t jumpscareAudioVolumeNew) {
		Manager::get()->jumpscareAudioVolume = static_cast<float>(std::clamp<int>(static_cast<int>(jumpscareAudioVolumeNew), 0, 100)) / 100.f;
	});
	listenForSettingChanges<double>("jumpscareFadeOutTime", [](const double jumpscareFadeOutTimeNew) {
		Manager::get()->jumpscareFadeOutTime = std::clamp<float>(static_cast<float>(jumpscareFadeOutTimeNew), .01f, 15.f);
	});
	listenForSettingChanges<double>("jumpscareFadeOutDelay", [](const double jumpscareFadeOutDelayNew) {
		Manager::get()->jumpscareFadeOutDelay = std::clamp<float>(static_cast<float>(jumpscareFadeOutDelayNew), .0f, 15.f);
	});
	listenForSettingChanges<std::filesystem::path>("jumpscaresFolder", [](const std::filesystem::path&) {
		Manager::loadOtherJumpscares();
	});
	listenForSettingChanges<std::filesystem::path>("additionalJumpscaresFolder", [](const std::filesystem::path&) {
		Manager::loadOtherJumpscares();
	});
	listenForSettingChanges<bool>("randomizeJumpscares", [](const bool randomizeJumpscares) {
		Manager::loadOtherJumpscares();
		Manager::get()->randomizeJumpscares = randomizeJumpscares;
		if (UniversalJumpscareSprite* unjus = Utils::getUNJUS(); unjus && randomizeJumpscares) {
			unjus->unscheduleAllSelectors();
			unjus->setOpacity(0);
			Utils::replaceUNJUS(unjus, Manager::get());
		} else if (!randomizeJumpscares) {
			Utils::removeUNJUS();
			Utils::addUNJUS(Mod::get()->getSettingValue<std::filesystem::path>("jumpscareImage"));
		}
	});
	listenForSettingChanges<bool>("logging", [](const bool logging) {
		Mod::get()->setLoggingEnabled(logging);
	});
}