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

void UniversalJumpscareSprite::canYouHearMeCallingFromWayTheFrickDownHere(float dt) {
	UniversalJumpscareSprite* unjus = Utils::getUNJUS();
	if (!unjus) return;

	if (!Utils::modEnabled()) return unjus->setVisible(false);

	const bool wasVisible = unjus->isVisible();
	Utils::handleUNJUS();
	const bool currentlyVisible = unjus->isVisible();

	Manager* manager = Manager::get();

	if (unjus->numberOfRunningActions() > 0 && !unjus->getActionByTag(20260104)) {
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
			if (animSprite->getCurrentFrame() > animSprite->getFrameCount() - 3) {
				unjus->stopAllActions();
				animSprite->stop();
				animSprite->setCurrentFrame(animSprite->getFrameCount() - 1); // stop on last frame to avoid visually jarring transitons
				unjus->runAction(CCFadeOut::create(manager->jumpscareFadeOutTime))->setTag(20260104);
			}
		}
		return;
	}

	if (manager->randomizeJumpscares && unjus->getTag() == 20260104) {
		if (unjus->getOpacity() > 0) return;

		unjus->setTag(20260105);
		Loader::get()->queueInMainThread([unjus, manager]() {
			Utils::replaceUNJUS(unjus, manager);
		});

		return;
	}

	manager->timePassed += dt;
	if (!unjus->isVisible() || manager->timePassed < manager->probabilityFrequency) return;
	if (Manager::shouldNotJumpscare()) return;

	manager->channel->stop();
	if (manager->unjusIsAnimated) {
		imgp::AnimatedSprite* animSprite = imgp::AnimatedSprite::from(manager->unjus);
		animSprite->setForceLoop(std::make_optional<bool>(false));
		animSprite->setCurrentFrame(0);
		animSprite->play();
	}
	unjus->setOpacity(255);
	if (manager->randomizeJumpscares) unjus->setTag(20260104);
	manager->timePassed = 0.;

	CCDelayTime* delay = CCDelayTime::create(!manager->unjusIsAnimated ? manager->jumpscareFadeOutDelay : 2123456789.f);
	CCFadeOut* fadeOut = CCFadeOut::create(!manager->unjusIsAnimated ? manager->jumpscareFadeOutTime : 2123456789.f);
	CCSequence* seqnce = CCSequence::create(delay, fadeOut, nullptr);
	unjus->runAction(seqnce);

	if (!manager->sound || !std::filesystem::exists(manager->currentAudio)) return log::info("manager->sound IS NULL REPORT THIS");
	manager->system->playSound(manager->sound, nullptr, false, &manager->channel);
	manager->channel->setVolume(manager->jumpscareAudioVolume);
}
