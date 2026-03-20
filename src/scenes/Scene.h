//
// Created by jhone on 18/03/2026.
//

#ifndef SCENE_H
#define SCENE_H
#include <SDL3/SDL_render.h>
#include "entt/entt.hpp"
#include "systems/system.h"
#include <memory>

class App;

class Scene {

protected:
    entt::registry &registry;
    std::vector<std::unique_ptr<System>> systems;
    App *app{nullptr};

public:
    virtual ~Scene() = default;

    explicit Scene(entt::registry &registry, App *app);

    virtual void init(SDL_Renderer* renderer) =0;

    virtual void input_event(SDL_Event *event) =0;

    virtual void update() =0;

    virtual void render(SDL_Renderer &render) =0;
};


#endif //SCENE_H
