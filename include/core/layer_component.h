// copyright 2025 swaroop.

#ifndef VK_SHADER_EXP_LAYER_COMPONENT_H
#define VK_SHADER_EXP_LAYER_COMPONENT_H

#include <string>

class EngineObject;
class Engine;

class LayerComponent {
public:
    explicit LayerComponent(EngineObject* initializerObj, const std::string& name = "RenderLayer");
    virtual ~LayerComponent() = default;

    virtual void onAttach() {}
    virtual void onDetach() {}
    virtual void onUpdate(float deltaTime) {}
    virtual void onRender() {}

    void setEngine(Engine* engineRef);
    
    // Getters for accessing the engine/parent from outside or derived classes
    Engine* getEngine() const;
    EngineObject* getParent() const;
    const std::string& getName() const;

protected:
    Engine* engine = nullptr;
    EngineObject* parent = nullptr;
    std::string debugName = "RenderLayer";
};

#endif // VK_SHADER_EXP_LAYER_COMPONENT_H