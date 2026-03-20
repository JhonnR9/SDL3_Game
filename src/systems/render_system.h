//
// Created by jhone on 18/03/2026.
//

#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H
#include "System.h"
#include <SDL3/SDL.h>
#include <map>

class RenderSystem final : public System{
    SDL_Renderer* renderer{nullptr};
    std::map<std::string, SDL_Texture*> textures;

    void load_textures();
public:
    RenderSystem(entt::registry &registry, SDL_Renderer* renderer);
    ~RenderSystem() override;

    void run(float dt) override;
};



#endif //RENDER_SYSTEM_H
