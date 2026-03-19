//
// Created by jhone on 18/03/2026.
//

#include "my_scene.h"
#include "components.h"

MyScene::MyScene(entt::registry *registry, App *app): Scene(registry, app) {
}

void MyScene::init() {

    entt::entity player = registry->create();

    Transform transform;
    Sprite sprite;

    sprite.texture_path = "assets/image.png";
    transform.scale = Vector2(1.f, 1.f);

    registry->emplace<Transform>(player, transform);
    registry->emplace<Sprite>(player, sprite);

}

void MyScene::render(SDL_Renderer *renderer) {

}

void MyScene::update() {
}

void MyScene::input_event(SDL_Event *event) {
}

