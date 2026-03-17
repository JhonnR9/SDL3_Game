#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cglm/cglm.h>

#include "label.h"
#include "renderer_2_d.h"

static Renderer2D *renderer = nullptr;
static AppContext *ctx = nullptr;
static Texture2D *texture = nullptr;
static Texture2D *texture2 = nullptr;
static Texture2D *texture3 = nullptr;
static Label *label = nullptr;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;

    ctx = new AppContext();

    ctx->window = SDL_CreateWindow("SDL3 GPU ", 640, 480, SDL_WINDOW_RESIZABLE);
    ctx->device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);

    if (!ctx->device || !SDL_ClaimWindowForGPUDevice(ctx->device, ctx->window)) return SDL_APP_FAILURE;
    if (!TTF_Init()) {
        SDL_Log("TTF init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    renderer = new Renderer2D();
    renderer->init(ctx);

    texture = renderer->load_texture("assets/image.png");
    texture2 = renderer->load_texture("assets/enemy2.png");
    texture3 = renderer->load_texture("assets/miyabi.jpeg");
    label = new Label("Texto pro", renderer);
    label->set_text_color(SDL_Color{ 255, 240, 0 });
    label->set_font_size(48);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    renderer->begin_draw();

    renderer->draw_texture(
        texture, Vector2(100.f, 50.f), glm_rad(0.f), Vector2(1.0f, 1.0f),
        Vector2(texture->width / 8, texture->height / 8)
    );

    renderer->draw_texture(
        texture2, Vector2(50.f, 400.f), glm_rad(0.f), Vector2(1.0f, 1.0f),
        Vector2(texture2->width, texture2->height)
    );

    renderer->draw_texture(
        texture3, Vector2(180.f, 250.f), glm_rad(0.f), Vector2(.3f, .3f),
        Vector2(texture3->width, texture3->height)
    );

    if (Texture2D *texture = label->get_data().font_texture) {
        renderer->draw_texture(
            label->get_data().font_texture, Vector2(label->get_data().font_dst.x, label->get_data().font_dst.y),
            glm_rad(0.f), Vector2(1.f, 1.f),
            Vector2(label->get_data().font_dst.w, label->get_data().font_dst.h)
        );
    }


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

    delete ctx;
    delete renderer;
    delete texture;
    delete texture2;
    delete texture3;
    delete label;
    SDL_Quit();
}
