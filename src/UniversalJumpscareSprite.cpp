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
	if (!unjus || unjus->getTag() == 20260105) return; // abort if there is no sprite OR if sprite is about to be removed

	if (!Utils::modEnabled()) return unjus->setVisible(false);

	const bool wasVisible = unjus->isVisible();
	Utils::handleUNJUS();
	const bool currentlyVisible = unjus->isVisible();
	unjus->setUserFlag("should-hide"_spr, wasVisible != currentlyVisible);

	Manager* manager = Manager::get();

	if (unjus->shouldStop()) return;
	if (unjus->randomizeOrStillNonZeroOpacity()) return;

	manager->timePassed += dt;
	if (!unjus->isVisible() || manager->timePassed < manager->probabilityFrequency) return;
	if (Manager::shouldNotJumpscare(JumpscareType::RandomTimer)) return;

	unjus->play(JumpscareType::RandomTimer);
}

bool UniversalJumpscareSprite::shouldStop() {
	UniversalJumpscareSprite* unjus = Utils::getUNJUS();
	if (unjus->numberOfRunningActions() > 0 && !unjus->getActionByTag(20260104)) {
		unjus->stop();
		return true;
	}
	return false;
}

bool UniversalJumpscareSprite::randomizeOrStillNonZeroOpacity() {
	Manager* manager = Manager::get();
	UniversalJumpscareSprite* unjus = Utils::getUNJUS();
	if (manager->randomizeJumpscares && unjus->getTag() == 20260104) {
		if (unjus->getOpacity() > 0) return true;

		unjus->setTag(20260105); // flag sprite for removal
		Loader::get()->queueInMainThread([unjus, manager]() {
			Utils::replaceUNJUS(unjus, manager);
		});

		return true;
	}
	return false;
}

void UniversalJumpscareSprite::play(JumpscareType jumpscare) {
	Manager* manager = Manager::get();
	UniversalJumpscareSprite* unjus = Utils::getUNJUS();

	manager->channel->stop();
	if (manager->unjusIsAnimated) {
		imgp::AnimatedSprite* animSprite = imgp::AnimatedSprite::from(manager->unjus);
		animSprite->setForceLoop(std::make_optional<bool>(false));
		animSprite->setCurrentFrame(0);
		animSprite->play();
	}
	unjus->setOpacity(255);
	if (manager->randomizeJumpscares) unjus->setTag(20260104);
	if (jumpscare == JumpscareType::RandomTimer) manager->timePassed = 0.;

	CCDelayTime* delay = CCDelayTime::create(!manager->unjusIsAnimated ? manager->jumpscareFadeOutDelay : 2123456789.f);
	CCFadeOut* fadeOut = CCFadeOut::create(!manager->unjusIsAnimated ? manager->jumpscareFadeOutTime : 2123456789.f);
	CCSequence* seqnce = CCSequence::create(delay, fadeOut, nullptr);
	unjus->runAction(seqnce);

	std::error_code ec;
	if (!manager->sound || !std::filesystem::exists(manager->currentAudio, ec)) {
		if (Mod::get()->isLoggingEnabled()) log::info("manager->sound not found. if manager->currentAudio ({}) is NOT empty, report this.", geode::utils::string::pathToString(manager->currentAudio));
		return;
	}

	manager->system->playSound(manager->sound, nullptr, false, &manager->channel);
	manager->channel->setVolume(manager->jumpscareAudioVolume);
}

void UniversalJumpscareSprite::stop() {
	Manager* manager = Manager::get();
	UniversalJumpscareSprite* unjus = Utils::getUNJUS();

	if (manager->forceHideIfJumpscareStillActive && unjus->getUserFlag("should-hide"_spr)) {
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
	unjus->setUserFlag("should-hide"_spr, false);
}