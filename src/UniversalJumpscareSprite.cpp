#include <prevter.imageplus/include/api.hpp>
#include "UniversalJumpscareSprite.hpp"
#include "Utils.hpp"

using namespace geode::prelude;

UniversalJumpscareSprite* UniversalJumpscareSprite::create(const char* imageFile) {
	auto* ret = new UniversalJumpscareSprite();
	if (ret && ret->initWithFile(imageFile)) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool UniversalJumpscareSprite::initWithFile(const char* content) {
	if (!CCSprite::initWithFile(content)) return false;
	return true;
}

void UniversalJumpscareSprite::update(float dt) {
	UniversalJumpscareSprite* unjus = Utils::getUNJUS();
	if (!unjus) return;

	if (!Utils::modEnabled()) return unjus->setVisible(false);

	const bool wasVisible = unjus->isVisible();
	Utils::handleUNJUS();
	const bool currentlyVisible = unjus->isVisible();

	Manager* manager = Manager::get();
	if (unjus->numberOfRunningActions() > 0) {
		if (manager->forceHideIfJumpscareStillActive && wasVisible != currentlyVisible) {
			unjus->stopAllActions();
			unjus->setOpacity(0);
			manager->channel->stop();
			if (manager->unjusIsAnimated) {
				imgp::AnimatedSprite* animSprite = imgp::AnimatedSprite::from(unjus);
				animSprite->setCurrentFrame(0);
				animSprite->stop();
			}
		} else if (manager->unjusIsAnimated) {
			imgp::AnimatedSprite* animSprite = imgp::AnimatedSprite::from(unjus);
			if (animSprite->getCurrentFrame() == animSprite->getFrameCount() - 1) {
				unjus->stopAllActions();
				animSprite->stop();
				animSprite->setCurrentFrame(animSprite->getFrameCount() - 1);
				unjus->runAction(CCFadeOut::create(manager->jumpscareFadeOutTime));
			}
		}
		return;
	}

	if (!unjus->isVisible() || Manager::shouldNotJumpscare()) return;
	if (manager->unjusIsAnimated) {
		imgp::AnimatedSprite* animSprite = imgp::AnimatedSprite::from(manager->unjus);
		animSprite->setCurrentFrame(0);
		animSprite->play();
	}
	unjus->setOpacity(255);

	CCDelayTime* delay = CCDelayTime::create(!manager->unjusIsAnimated ? manager->jumpscareFadeOutDelay : 2123456789.f);
	CCFadeOut* fadeOut = CCFadeOut::create(!manager->unjusIsAnimated ? manager->jumpscareFadeOutTime : 2123456789.f);
	CCSequence* seqnce = CCSequence::create(delay, fadeOut, nullptr);
	unjus->runAction(seqnce);

	manager->system->playSound(manager->sound, nullptr, false, &manager->channel);
	manager->channel->setVolume(manager->jumpscareAudioVolume);
}