#include "app.h"
#include <SDL3_ttf/SDL_ttf.h>

#include "my_scene.h"
#include "render_system.h"
#include "scene.h"

App::App() {
    this->ctx = new AppContext();
    ctx->window = nullptr;
    ctx->renderer = nullptr;
}

SDL_AppResult App::init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
    if (!TTF_Init()) return SDL_APP_FAILURE;

    ctx->window = SDL_CreateWindow("SDL3 Renderer", 1366, 768, SDL_WINDOW_RESIZABLE);
    ctx->renderer = SDL_CreateRenderer(ctx->window, nullptr);
    SDL_SetRenderLogicalPresentation(ctx->renderer, 1366, 768, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    if (!ctx->window || !ctx->renderer) return SDL_APP_FAILURE;

    registry = new entt::registry();
    auto system = std::make_unique<RenderSystem>(registry, ctx->renderer);
    systems.push_back(std::move(system));

    set_scene(new MyScene(registry, this));

    return SDL_APP_CONTINUE;
}

void App::input_event(SDL_Event *event) {
    if (scene) {
        scene->input_event(event);
    }
}

void App::render() {
    if (scene) {
        scene->render(ctx->renderer);
    }

    for (auto &system: systems) {
        SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
        SDL_RenderClear(ctx->renderer);
        system->run(0.01f);
        SDL_RenderPresent(ctx->renderer);
    }
}

void App::set_scene(Scene *scene) {
    if (scene == this->scene) {
        return;
    }

    if (!scene) {
        if (this->scene) {
            delete this->scene;
            this->scene = nullptr;
            return;
        }

        return;
    }
    if (this->scene) {
        delete this->scene;
        this->scene = nullptr;
    }

    this->scene = scene;
    this->scene->init();
}

App::~App() {
    delete scene;
    delete registry;

    if (ctx) {
        if (ctx->renderer) {
            SDL_DestroyRenderer(ctx->renderer);
        }

        if (ctx->window) {
            SDL_DestroyWindow(ctx->window);
        }

        delete ctx;
    }

    TTF_Quit();
    SDL_Quit();
}
