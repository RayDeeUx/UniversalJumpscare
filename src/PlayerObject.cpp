#include <Geode/modify/PlayerObject.hpp>
#include "UniversalJumpscareSprite.hpp"
#include "Manager.hpp"
#include "Utils.hpp"

class $modify(MyPlayerObject, PlayerObject) {
	bool pushButton(PlayerButton button) {
		bool ret = PlayerObject::pushButton(button);
		UniversalJumpscareSprite* unjus = Utils::getUNJUS();

		if (Utils::modEnabled() && this->isPlayer1() && button == PlayerButton::Jump && this->m_gameLayer && !this->m_gameLayer->m_isEditor && unjus->isVisible() && !unjus->shouldStop() && !Manager::shouldNotJumpscare(JumpscareType::GameplayClick)) {
			unjus->play(JumpscareType::GameplayClick); // the enum is there to prevent timer from being unintentionally reset
		}

		return ret;
	}
	void playerDestroyed(bool noEffects) {
		PlayerObject::playerDestroyed(noEffects);
		UniversalJumpscareSprite* unjus = Utils::getUNJUS();

		if (Utils::modEnabled() && this->isPlayer1() && this->m_isDead && this->m_gameLayer && !this->m_gameLayer->m_isEditor && unjus->isVisible() && !unjus->shouldStop() && !Manager::shouldNotJumpscare(JumpscareType::GameplayDeath)) {
			unjus->play(JumpscareType::GameplayDeath); // the enum is there to prevent timer from being unintentionally reset
		}

		return ret;
	}
};