#pragma once

class UniversalJumpscareSprite final : public cocos2d::CCSprite {
protected:
	bool initWithFile(const char*) override;
	void update(float) override;
public:
	static UniversalJumpscareSprite* create(const char*);
};