#pragma once
#include "UniversalJumpscareSprite.hpp"
#include "Manager.hpp"

using namespace geode::prelude;

namespace Utils {
	template<class T> T getSetting(std::string setting, T type);
	bool getBool(std::string setting);
	int64_t getInt(std::string setting);
	double getDouble(std::string setting);
	std::string getString(std::string setting);
	ccColor3B getColor(std::string setting);
	ccColor4B getColorAlpha(std::string setting);
	bool modEnabled();
	
	bool isModLoaded(std::string modID);
	Mod* getMod(std::string modID);
	std::string getModVersion(Mod* mod);

	UniversalJumpscareSprite* getUNJUS(cocos2d::CCScene* scene = CCDirector::get()->getRunningScene());
	void handleUNJUS();
	void addUNJUS();
	void removeUNJUS();
	void setUNJUSScale(UniversalJumpscareSprite* unjus, cocos2d::CCSize win = CCDirector::get()->getWinSize());
	void setupUNJUS(UniversalJumpscareSprite* unjus);
	void addChroma(UniversalJumpscareSprite* unjus = Utils::getUNJUS());
	std::string chooseFontFile(int64_t font);
}