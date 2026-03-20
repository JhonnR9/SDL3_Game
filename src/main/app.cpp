#include "app.h"

#include <SDL3_ttf/SDL_ttf.h>
#include "scenes/my_scene.h"
#include "systems/render_system.h"
#include "scenes/scene.h"

App::App() {
    this->ctx = std::make_unique<AppContext>();

    ctx->window = nullptr;
    ctx->renderer = nullptr;
}

SDL_AppResult App::init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
    if (!TTF_Init()) return SDL_APP_FAILURE;

    ctx->window = SDL_CreateWindow("SDL3 Renderer", 840, 480, SDL_WINDOW_RESIZABLE);
    ctx->renderer = SDL_CreateRenderer(ctx->window, nullptr);
    SDL_SetRenderLogicalPresentation(ctx->renderer, 840, 480, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    if (!ctx->window || !ctx->renderer) return SDL_APP_FAILURE;

    registry = std::make_unique<entt::registry>();

    asset_manager = std::make_unique<AssetManager>(ctx->renderer);
    registry->ctx().emplace<AssetManager*>(asset_manager.get());


    set_scene(std::make_unique<MyScene>(*registry.get(), this));

    return SDL_APP_CONTINUE;
}

void App::input_event(SDL_Event *event) {
    if (current_scene) {
        current_scene->input_event(event);
    }
}

void App::render() {
    const uint64_t current_time = SDL_GetTicks();
    game_time.delta_time = (current_time - game_time.last_time) / 1000.0f; // convert to seconds

    asset_manager->process_gpu_uploads();


    if (current_scene) {
        current_scene->render(*ctx->renderer);
    }
}

void App::set_scene(std::unique_ptr<Scene> new_scene) {
    if (new_scene.get() == this->current_scene.get()) {
        return;
    }

    this->current_scene = std::move(new_scene);
    this->current_scene->init(ctx->renderer);
}

App::~App() {
    asset_manager->shutdown();

    if (ctx) {
        if (ctx->renderer) {
            SDL_DestroyRenderer(ctx->renderer);
        }

        if (ctx->window) {
            SDL_DestroyWindow(ctx->window);
        }
    }

    TTF_Quit();
    SDL_Quit();
}
