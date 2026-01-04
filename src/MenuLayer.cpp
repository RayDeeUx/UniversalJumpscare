#include <Geode/modify/MenuLayer.hpp>
#include "Manager.hpp"
#include "Utils.hpp"

using namespace geode::prelude;

class $modify(MyMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;

		Manager* manager = Manager::get();
		if (manager->calledAlready) return true;
		manager->calledAlready = true;

		if (!Utils::modEnabled()) return true;
		if (Utils::getUNJUS()) return true;

		if (manager->randomizeJumpscares) {
			auto iterator = Manager::pickRandomJumpscare(manager->jumpscares);
			if (iterator == manager->jumpscares.end()) {
				Utils::addUNJUS();
			} else {
				const auto&[imageFile, audioFile] = *iterator;
				manager->channel->stop();
				FMOD::Sound* originalSound = manager->sound;

				manager->currentImage = std::filesystem::path{};
				manager->currentAudio = std::filesystem::path{};

				if (std::filesystem::exists(imageFile)) Utils::addUNJUS(imageFile);
				if (std::filesystem::exists(audioFile) && Manager::acceptableAudioFileExtension(audioFile)) {
					manager->currentAudio = audioFile;
					manager->system->createSound(geode::utils::string::pathToString(audioFile).c_str(), FMOD_DEFAULT, nullptr, &manager->sound);
				}
				if (originalSound) originalSound->release();
			}
		} else {
			Utils::addUNJUS();
		}

		return true;
	}
};