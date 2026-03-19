
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include "app.h"

static App* app = nullptr;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    app = new App();
    return app->init();
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    app->render();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
    app->input_event(event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    delete app;
}
