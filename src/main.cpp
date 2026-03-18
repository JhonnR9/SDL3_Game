#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#include "label.h"

static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;

static SDL_Texture *texture = nullptr;
static SDL_Texture *texture2 = nullptr;
static SDL_Texture *texture3 = nullptr;
static Label* label = nullptr;


SDL_Texture* load_texture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        SDL_Log("Erro ao carregar imagem (%s): %s", path, SDL_GetError());
        return nullptr;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);

    if (!tex) {
        SDL_Log("Erro ao criar textura: %s", SDL_GetError());
    }

    return tex;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
    if (!TTF_Init()) return SDL_APP_FAILURE;


    window = SDL_CreateWindow("SDL3 Renderer", 640, 480, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, NULL);

    if (!window || !renderer) return SDL_APP_FAILURE;

    texture  = load_texture("assets/image.png");
    texture2 = load_texture("assets/enemy2.png");
    texture3 = load_texture("assets/miyabi.jpeg");



    if (!texture || !texture2 || !texture3) {
        SDL_Log("Erro ao carregar texturas!");
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderLogicalPresentation(renderer, 640, 480, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    label = new Label("Hello World!", renderer);
    label->set_font_size(48);
    label->set_text_color(SDL_Color(255,100,255,255));

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    float w, h;

    SDL_GetTextureSize(texture, &w, &h);
    SDL_FRect dst1 = {100.f, 50.f, w / 8.f, h / 8.f};
    SDL_RenderTexture(renderer, texture, NULL, &dst1);

    SDL_GetTextureSize(texture2, &w, &h);
    SDL_FRect dst2 = {50.f, 400.f, w, h};
    SDL_RenderTexture(renderer, texture2, NULL, &dst2);

    SDL_GetTextureSize(texture3, &w, &h);
    SDL_FRect dst3 = {180.f, 250.f, w * 0.3f, h * 0.3f};
   SDL_RenderTexture(renderer, texture3, NULL, &dst3);

    auto a = label->get_data().font_dst;
    if (label) {
        SDL_RenderTexture(renderer, label->get_data().font_texture, NULL, &a);
    }

    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    if (texture) SDL_DestroyTexture(texture);
    if (texture2) SDL_DestroyTexture(texture2);
    if (texture3) SDL_DestroyTexture(texture3);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);

    TTF_Quit();
    SDL_Quit();
}
