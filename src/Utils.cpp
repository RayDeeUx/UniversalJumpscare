#include <prevter.imageplus/include/api.hpp>
#include "Utils.hpp"

using namespace geode::cocos;

namespace Utils {

	bool getBool(const std::string_view setting) { return Mod::get()->getSettingValue<bool>(setting); }
	
	int64_t getInt(const std::string_view setting) { return Mod::get()->getSettingValue<int64_t>(setting); }
	
	double getDouble(const std::string_view setting) { return Mod::get()->getSettingValue<double>(setting); }

	std::string getString(const std::string_view setting) { return Mod::get()->getSettingValue<std::string>(setting); }

	ccColor3B getColor(const std::string_view setting) { return Mod::get()->getSettingValue<ccColor3B>(setting); }

	ccColor4B getColorAlpha(const std::string_view setting) { return Mod::get()->getSettingValue<ccColor4B>(setting); }

	bool modEnabled() { return getBool("enabled"); }
	
	bool isModLoaded(const std::string& modID) { return Loader::get()->isModLoaded(modID); }

	Mod* getMod(const std::string& modID) { return Loader::get()->getLoadedMod(modID); }

	std::string getModVersion(const Mod* mod) { return mod->getVersion().toNonVString(); }

	UniversalJumpscareSprite* getUNJUS() { return Manager::get()->unjus; }
	
	void handleUNJUS() {
		if (!Utils::modEnabled()) return Utils::removeUNJUS();
		UniversalJumpscareSprite* unjus = Utils::getUNJUS();
		const Manager* manager = Manager::get();
		const GJBaseGameLayer* gjbgl = GJBaseGameLayer::get();
		const LevelEditorLayer* lel = LevelEditorLayer::get();
		const PlayLayer* pl = PlayLayer::get();
		if (unjus && manager->hideEverywhereElse && !pl && !lel) unjus->setVisible(false);
		if (unjus && manager->hideInLevelEditorLayer && lel) return unjus->setVisible(false);
		if (!unjus && manager->hideInLevelEditorLayer && !lel) {
			Utils::addUNJUS();
			unjus = Utils::getUNJUS();
		}
		if (gjbgl && unjus) {
			if (pl) {
				const std::string& playLayerVisibility = manager->visibilityInPlayLayer;
				if (playLayerVisibility.starts_with("always ")) {
					if (playLayerVisibility == "always visible") unjus->setVisible(true);
					else if (playLayerVisibility == "always hidden") unjus->setVisible(false);
				} else if (playLayerVisibility.starts_with("only ") && playLayerVisibility.ends_with(" when dead")) {
					if (const PlayerObject* player = pl->m_player1) {
						const bool onlyShowWhenDead = playLayerVisibility == "only show when dead";
						const bool onlyHideWhenDead = playLayerVisibility == "only hide when dead";
						if (player->m_isDead) {
							if (onlyShowWhenDead) unjus->setVisible(true);
							else if (onlyHideWhenDead) unjus->setVisible(false);
						} else {
							if (onlyShowWhenDead) unjus->setVisible(false);
							else if (onlyHideWhenDead) unjus->setVisible(true);
						}
					}
				}
			} else if (lel) {
				if (manager->hideInLevelEditorLayer) return unjus->setVisible(false);
				Utils::addUNJUS();
			}
		} else if (unjus) unjus->setVisible(!manager->hideEverywhereElse);
	}

	void addUNJUS(const std::filesystem::path& imagePath) {
		if (UniversalJumpscareSprite* unjus = Utils::getUNJUS(); unjus) return unjus->setVisible(true);
		UniversalJumpscareSprite* newSprite = UniversalJumpscareSprite::create(geode::utils::string::pathToString(imagePath).c_str());
		if (!newSprite) return log::info("UNJUS addition operation failed, node was not created properly");
		Utils::setupUNJUS(newSprite);
		CCScene::get()->addChild(newSprite);
		SceneManager::get()->keepAcrossScenes(newSprite);
		newSprite->setVisible(true);
		newSprite->setOpacity(0);
		newSprite->setPosition(CCScene::get()->getContentSize() / 2.f);
		Manager::get()->unjus = newSprite;
		Manager::calculateProbability(Utils::getInt("probabilityNumerator"), Utils::getInt("probabilityDenominator"));
		if (Utils::getBool("logging")) log::info("UNJUS added");
	}

	void removeUNJUS() {
		UniversalJumpscareSprite* unjus = Utils::getUNJUS();
		if (!unjus) return;
		SceneManager::get()->forget(unjus);
		unjus->removeMeAndCleanup();
		Manager::get()->unjus = nullptr;
		if (Utils::getBool("logging")) log::info("UNJUS removed");
	}

	void replaceUNJUS(UniversalJumpscareSprite* unjus, Manager *manager) {
		auto iterator = Manager::pickRandomJumpscare(manager->jumpscares);
		if (iterator == manager->jumpscares.end()) return unjus->setTag(-1);

		const auto&[imageFile, audioFile] = *iterator;

		Utils::removeUNJUS();
		manager->channel->stop();
		manager->sound->release();

		Utils::addUNJUS(imageFile);
		if (std::filesystem::exists(audioFile) && Manager::acceptableAudioFileExtension(audioFile)) manager->system->createSound(geode::utils::string::pathToString(audioFile).c_str(), FMOD_DEFAULT, nullptr, &manager->sound);
	}

	void setUNJUSScale(UniversalJumpscareSprite* unjus, const CCSize& win) {
		if (!unjus) return;
		const CCSize replacementSize = unjus->getContentSize();
		const float yRatio = win.height / replacementSize.height;
		const float xRatio = win.width / replacementSize.width;
		const std::string& fillMode = utils::string::toLower(Utils::getString("jumpscareImageFillMode"));

		if (fillMode == "fit to screen") unjus->setScale(std::min(xRatio, yRatio));
		else if (fillMode == "fill screen") unjus->setScale(std::max(xRatio, yRatio));
		else if (fillMode == "stretch to fill screen") {
			unjus->setScaleX(xRatio);
			unjus->setScaleY(yRatio);
		} else if (fillMode == "leave as is") {
			unjus->setScale(1.f);
		} else {
			unjus->setScale(std::clamp<float>(static_cast<float>(Utils::getDouble("jumpscareImageScale")), .01f, 5.f));
		}
	}

	void setupUNJUS(UniversalJumpscareSprite* unjus) {
		Utils::setUNJUSScale(unjus);
		unjus->setTag(10162025);
		unjus->setID("universal-jumpscare-sprite"_spr);
		Manager::get()->unjusIsAnimated = imgp::AnimatedSprite::from(unjus)->isAnimated();
		unjus->schedule(schedule_selector(UniversalJumpscareSprite::canYouHearMeCallingFromWayTheFrickDownHere), static_cast<float>(Manager::get()->probabilityFrequency));
	}

}