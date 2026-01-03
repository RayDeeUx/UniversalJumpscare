#include "UniversalJumpscareSprite.hpp"
#include "Manager.hpp"
#include "Utils.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
	Manager::get()->system->createSound(geode::utils::string::pathToString(Mod::get()->getSettingValue<std::filesystem::path>("jumpscareAudio")).c_str(), FMOD_DEFAULT, nullptr, &Manager::get()->sound);
	Manager::get()->channel->setVolume(static_cast<float>(std::clamp<int>(static_cast<int>(Mod::get()->getSettingValue<int64_t>("jumpscareAudioVolume")), 0, 100)) / 100.f);

	listenForSettingChanges<bool>("enabled", [](bool isEnabled) {
		UniversalJumpscareSprite* unjus = Utils::getUNJUS();
		if (!isEnabled && unjus) return Utils::removeUNJUS();
		if (isEnabled && !unjus) return Utils::addUNJUS();
	});
	listenForSettingChanges<std::filesystem::path>("jumpscareImage", [](const std::filesystem::path& path) {
		Utils::removeUNJUS();
		Utils::addUNJUS(path);
	});
	listenForSettingChanges<int64_t>("probabilityNumerator", [](int64_t newNumerator) {
		Manager::calculateProbability(newNumerator, Mod::get()->getSettingValue<int64_t>("probabilityDenominator"));
	});
	listenForSettingChanges<int64_t>("probabilityDenominator", [](int64_t newDenominator) {
		Manager::calculateProbability(Mod::get()->getSettingValue<int64_t>("probabilityNumerator"), newDenominator);
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
		Manager::get()->system->createSound(geode::utils::string::pathToString(jumpscareAudioNew).c_str(), FMOD_DEFAULT, nullptr, &Manager::get()->sound);
	});
	listenForSettingChanges<int64_t>("jumpscareAudioVolume", [](int64_t jumpscareAudioVolumeNew) {
		Manager::get()->jumpscareAudioVolume = static_cast<float>(std::clamp<int>(static_cast<int>(jumpscareAudioVolumeNew), 0, 100)) / 100.f;
	});
	listenForSettingChanges<double>("jumpscareFadeOutTime", [](double jumpscareFadeOutTimeNew) {
		Manager::get()->jumpscareFadeOutTime = std::clamp<float>(static_cast<float>(jumpscareFadeOutTimeNew), .01f, 15.f);
	});
	listenForSettingChanges<double>("jumpscareFadeOutDelay", [](double jumpscareFadeOutDelayNew) {
		Manager::get()->jumpscareFadeOutDelay = std::clamp<float>(static_cast<float>(jumpscareFadeOutDelayNew), .0f, 15.f);
	});
}