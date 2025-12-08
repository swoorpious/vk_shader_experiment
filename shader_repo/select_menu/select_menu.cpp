// copyright 2025 swaroop.

#include "select_menu.h"
#include "select_menu_layer.h"
// #include <plasma_ball/plasma.h>
#include <engine.h>

SelectMenuObject::SelectMenuObject(Engine* e): EngineObject(e) {
}

void SelectMenuObject::onSetup() {
    EngineObject::onSetup();
    // this->registerClass<PlasmaBallObject>("Plasma Ball by Xor Dev");
    
    pushLayer(new SelectMenuLayer(this));
}

void SelectMenuObject::update(float deltaTime) {
    EngineObject::update(deltaTime);
}

void SelectMenuObject::render() {
    EngineObject::render();
}