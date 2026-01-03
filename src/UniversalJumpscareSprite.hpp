#pragma once

class UniversalJumpscareSprite : public cocos2d::CCSprite {
protected:
	bool init(const char*, const char*);

public:
	static UniversalJumpscareSprite* create(const char*);
	bool initWithFile(const char*);
	void update(float);
};