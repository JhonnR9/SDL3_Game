#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cglm/cglm.h>
#include "renderer_2_d.h"

static Renderer2D *renderer = nullptr;
static AppContext *ctx = nullptr;
static Texture2D *texture = nullptr;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;

    ctx = new AppContext();

    ctx->window = SDL_CreateWindow("SDL3 GPU Refactored", 640, 480, SDL_WINDOW_RESIZABLE);
    ctx->device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);

    if (!ctx->device || !SDL_ClaimWindowForGPUDevice(ctx->device, ctx->window)) return SDL_APP_FAILURE;

    renderer = new Renderer2D();
    renderer->init(ctx);

    texture = renderer->load_texture("assets/image.png");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {

    renderer->begin_draw();
    renderer->draw_texture(texture);
    renderer->end_draw();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    if (ctx) {
        SDL_WaitForGPUIdle(ctx->device);

        if (renderer)
            free(renderer);
        renderer = nullptr;

        if (ctx->window)
            SDL_DestroyWindow(ctx->window);
        ctx->window = nullptr;
    }


    SDL_free(ctx);
    SDL_Quit();
}
