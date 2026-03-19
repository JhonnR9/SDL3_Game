
#ifndef APP_H
#define APP_H

#include <entt/entt.hpp>
#include <SDL3/SDL_render.h>
#include "SDL3/SDL_init.h"
#include <vector>

#include "System.h"


typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
}AppContext;

class Scene;

class App {
    std::vector<std::unique_ptr<System>> systems;
    entt::registry* registry{nullptr};
    AppContext* ctx;
    Scene *scene{nullptr};


public:
    App() ;
    ~App();

    SDL_AppResult init();
    void input_event(SDL_Event *event);
    void render();
    void set_scene(Scene *scene);

};



#endif //APP_H
