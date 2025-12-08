// copyright 2025 swaroop.

#include <core/layer_component.h>
#include <core/engine_object.h>

LayerComponent::LayerComponent(EngineObject* initializerObj, const std::string& name) {
    if (initializerObj) {
        parent = initializerObj;
        engine = parent->getEngine();
    }

    if (!name.empty())
        debugName = name;
}

void LayerComponent::setEngine(Engine* engineRef) { 
    engine = engineRef; 
}

Engine* LayerComponent::getEngine() const { 
    return engine; 
}

EngineObject* LayerComponent::getParent() const { 
    return parent; 
}

const std::string& LayerComponent::getName() const { 
    return debugName; 
}