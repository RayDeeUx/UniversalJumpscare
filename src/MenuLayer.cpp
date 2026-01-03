#include <Geode/modify/MenuLayer.hpp>
#include "Manager.hpp"
#include "Utils.hpp"
#include <ctime>

using namespace geode::prelude;

class $modify(MyMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;

		Manager* manager = Manager::get();
		if (manager->calledAlready) return true;
		manager->calledAlready = true;

		if (!Utils::modEnabled()) return true;
		if (Utils::getUNJUS()) return true;

		Utils::addUNJUS();

		Manager::loadStuff();

		return true;
	}
};