#ifndef APP_H
#define APP_H

#include <entt/entt.hpp>
#include <SDL3/SDL_render.h>
#include "SDL3/SDL_init.h"

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} AppContext;


class Scene;

class App {
public:
    typedef struct {
        uint64_t last_time = 0;
        float delta_time = 0.0f;
    } GameTime;

private:
    std::unique_ptr<entt::registry> registry;

    std::unique_ptr<AppContext> ctx;

    std::unique_ptr<Scene> current_scene;
    GameTime game_time{};

public:
    App();

    ~App();

    SDL_AppResult init();

    void input_event(SDL_Event *event);

    void render();

    void set_scene(std::unique_ptr<Scene> new_scene);

    GameTime get_delta_time() {
        return game_time;
    }
};

#endif //APP_H
