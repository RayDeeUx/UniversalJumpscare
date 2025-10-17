#pragma once

#include "UniversalJumpscareSprite.hpp"
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

	static Manager* get() {
		if (!instance) instance = new Manager();
		return instance;
	}

	void loadStuff() {
		this->instance->hideInLevelEditorLayer = Mod::get()->getSettingValue<bool>("hideInLevelEditorLayer");
		this->instance->hideEverywhereElse = Mod::get()->getSettingValue<bool>("hideEverywhereElse");
		this->instance->forceHideIfJumpscareStillActive = Mod::get()->getSettingValue<bool>("forceHideIfJumpscareStillActive");
		this->instance->visibilityInPlayLayer = geode::utils::string::toLower(Mod::get()->getSettingValue<std::string>("visibilityInPlayLayer"));
		this->instance->jumpscareAudioVolume = static_cast<float>(std::clamp<int>(static_cast<int>(Mod::get()->getSettingValue<int64_t>("jumpscareAudioVolume")), 0, 100)) / 100.f;
		this->instance->jumpscareFadeOutTime = std::clamp<float>(static_cast<float>(Mod::get()->getSettingValue<double>("jumpscareFadeOutTime")), .01f, 15.f);
		this->instance->jumpscareFadeOutDelay = std::clamp<float>(static_cast<float>(Mod::get()->getSettingValue<double>("jumpscareFadeOutDelay")), .0f, 15.f);
	}

	static void calculateProbability(int64_t numerator, int64_t denominator) {
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
};