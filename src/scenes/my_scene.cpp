//
// Created by jhone on 18/03/2026.
//

#include "my_scene.h"
#include "components/components.h"
#include "main/app.h"
#include "systems/render_system.h"

MyScene::MyScene(entt::registry &registry, App *app): Scene(registry, app) {
}

void MyScene::init(SDL_Renderer* renderer) {
    const entt::entity player = registry.create();

    Transform transform;
    Sprite sprite;

    sprite.texture_path = "assets/image.png";
    transform.scale = Vector2(1.f, 1.f);

    registry.emplace<Transform>(player, transform);
    registry.emplace<Sprite>(player, sprite);

    auto system = std::make_unique<RenderSystem>(registry, renderer);
    systems.push_back(std::move(system));

}

void MyScene::render(SDL_Renderer &renderer) {
    SDL_SetRenderDrawColor(&renderer, 10, 10, 20, 255);
    SDL_RenderClear(&renderer);
    for (const auto &system: systems) {
        system->run(app->get_delta_time().delta_time);

    }
    SDL_RenderPresent(&renderer);

}

void MyScene::update() {
}

void MyScene::input_event(SDL_Event *event) {
}

