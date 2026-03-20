#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include "app.h"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
     const auto app = new App();

    const SDL_AppResult result = app->init();
    if (result != SDL_APP_CONTINUE) {
        delete app;
        return result;
    }

    *appstate = app;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    const auto app = static_cast<App *>(appstate);
    app->render();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    const auto app = static_cast<App *>(appstate);

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    app->input_event(event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    const App *app = static_cast<App *>(appstate);
    delete app;
}
