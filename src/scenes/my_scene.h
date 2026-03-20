//
// Created by jhone on 18/03/2026.
//

#ifndef MY_SCENE_H
#define MY_SCENE_H

#include "Scene.h"

class MyScene: public Scene{

public:
    explicit MyScene(entt::registry &registry, App* app);
    void render(SDL_Renderer &renderer) override ;
    void update() override;
    void input_event(SDL_Event *event) override;
    void init(SDL_Renderer* renderer) override;

};



#endif //MY_SCENE_H
