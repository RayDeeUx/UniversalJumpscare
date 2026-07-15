#pragma once

class UniversalJumpscareSprite final : public cocos2d::CCSprite {
protected:
	bool initWithFile(const char*) override;
public:
	static UniversalJumpscareSprite* create(const char*);
	void canYouHearMeCallingFromWayTheFrickDownHere(float);
	bool randomizeOrStillNonZeroOpacity();
	bool shouldStop();
	void play(int jumpscare);
	void stop();
};