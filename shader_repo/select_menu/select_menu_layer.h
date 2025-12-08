// copyright 2025 swaroop.

#ifndef VK_SHADER_ENGINE_SELECT_MENU_LAYER_H
#define VK_SHADER_ENGINE_SELECT_MENU_LAYER_H

#include <core/layer_component.h>

class SelectMenuLayer : public LayerComponent {
public:
    explicit SelectMenuLayer(EngineObject* parent);
    
    void onAttach() override;
    void onUpdate(float deltaTime) override;

private:
    float time_elapsed = 0.0f;
};

#endif // VK_SHADER_ENGINE_SELECT_MENU_LAYER_H