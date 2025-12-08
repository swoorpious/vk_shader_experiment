// copyright 2025 swaroop.

#ifndef VK_SHADER_ENGINE_SELECT_MENU_H
#define VK_SHADER_ENGINE_SELECT_MENU_H
#include <core/engine_object.h>
#include <unordered_map>
#include <string>
#include <functional>
#include <memory>


using namespace std;
class SelectMenuObject final : public EngineObject {
public:
    explicit SelectMenuObject(Engine* e);

    void onSetup() override;
    void update(float deltaTime) override;
    void render() override;

private:
    // time_elapsed moved to SelectMenuLayer
    unordered_map<string, function<unique_ptr<EngineObject>()>> repo_map;
    
    template<typename T>
    void registerClass(const string& name);
    
    unique_ptr<EngineObject> create(const string& name);
    
};


// select_manu repo_map functions
template <typename T>
void SelectMenuObject::registerClass(const string& name) {
    repo_map[name] = [this]() {
        return make_unique<T>(engine);
    };
}


inline unique_ptr<EngineObject> SelectMenuObject::create(const string& name) {
    const auto it = repo_map.find(name);
    if (it == repo_map.end()) return nullptr;
    return it->second();
}

#endif //VK_SHADER_ENGINE_SELECT_MENU_H