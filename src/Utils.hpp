#pragma once
#include "UniversalJumpscareSprite.hpp"
#include "Manager.hpp"

using namespace geode::prelude;

namespace Utils {
	template<class T> T getSetting(const std::string_view setting, T type) { return Mod::get()->getSettingValue<T>(setting); }
	bool getBool(const std::string_view);
	int64_t getInt(const std::string_view);
	double getDouble(const std::string_view);
	std::string getString(const std::string_view);
	ccColor3B getColor(const std::string_view);
	ccColor4B getColorAlpha(const std::string_view);
	bool modEnabled();
	
	bool isModLoaded(const std::string&);
	Mod* getMod(const std::string&);
	std::string getModVersion(const Mod*);

	bool acceptableAudioFileExtension(const std::filesystem::path&);
	bool acceptableImageFileExtension(const std::filesystem::path&);

	UniversalJumpscareSprite* getUNJUS();
	void handleUNJUS();
	void addUNJUS(const std::filesystem::path& imagePath = Mod::get()->getSettingValue<std::filesystem::path>("jumpscareSprite"));
	void removeUNJUS();
	void setUNJUSScale(UniversalJumpscareSprite* unjus, const cocos2d::CCSize& win = CCDirector::get()->getWinSize());
	void setupUNJUS(UniversalJumpscareSprite* unjus);
	void addChroma(UniversalJumpscareSprite* unjus = Utils::getUNJUS());
	std::string chooseFontFile(int64_t font);
	
	static const std::unordered_set<std::string_view> audioExtensions = {".mp3", ".wav", ".ogg", ".oga", ".flac"};
	static const std::unordered_set<std::string_view> imageExtensions = {".png", ".webp", ".gif", ".jpeg", ".jpg", ".apng", ".jxl", ".qoi"};
}