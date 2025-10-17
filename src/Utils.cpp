#include <prevter.imageplus/include/api.hpp>
#include "Manager.hpp"
#include "Utils.hpp"

#define SECONDS_PER_DAY 86400
#define SECONDS_PER_HOUR 3600
#define SECONDS_PER_MINUTE 60

using namespace geode::cocos;

namespace Utils {
	template<class T> T getSetting(std::string setting) { return Mod::get()->getSettingValue<T>(setting); }

	bool getBool(std::string setting) { return getSetting<bool>(setting); }
	
	int64_t getInt(std::string setting) { return getSetting<int64_t>(setting); }
	
	double getDouble(std::string setting) { return getSetting<double>(setting); }

	std::string getString(std::string setting) { return getSetting<std::string>(setting); }

	ccColor3B getColor(std::string setting) { return getSetting<ccColor3B>(setting); }

	ccColor4B getColorAlpha(std::string setting) { return getSetting<ccColor4B>(setting); }

	bool modEnabled() { return getBool("enabled"); }
	
	bool isModLoaded(std::string modID) { return Loader::get()->isModLoaded(modID); }

	Mod* getMod(std::string modID) { return Loader::get()->getLoadedMod(modID); }

	std::string getModVersion(Mod* mod) { return mod->getVersion().toNonVString(); }

	UniversalJumpscareSprite* getUNJUS(cocos2d::CCScene* scene) {
		return Manager::get()->unjus;
	}
	
	void handleUNJUS() {
		if (!Utils::modEnabled()) return Utils::removeUNJUS();
		UniversalJumpscareSprite* unjus = Utils::getUNJUS();
		Manager* manager = Manager::get();
		GJBaseGameLayer* gjbgl = GJBaseGameLayer::get();
		LevelEditorLayer* lel = LevelEditorLayer::get();
		PlayLayer* pl = PlayLayer::get();
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
					if (PlayerObject* player = pl->m_player1) {
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
				else Utils::addUNJUS();
			}
		} else if (unjus) unjus->setVisible(!manager->hideEverywhereElse);
	}

	void addUNJUS() {
		auto unjus = Utils::getUNJUS();
		if (unjus) return unjus->setVisible(true);
		auto newSprite = UniversalJumpscareSprite::create(geode::utils::string::pathToString(Mod::get()->getSettingValue<std::filesystem::path>("jumpscareImage")).c_str());
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
		auto unjus = Utils::getUNJUS();
		if (!unjus) return;
		SceneManager::get()->forget(unjus);
		unjus->removeMeAndCleanup();
		Manager::get()->unjus = nullptr;
		if (Utils::getBool("logging")) log::info("UNJUS removed");
	}

	void setUNJUSScale(UniversalJumpscareSprite* unjus, CCSize win) {
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
		GameManager::get()->schedule(static_cast<SEL_SCHEDULE>(&UniversalJumpscareSprite::updateWrapper));
	}

}
