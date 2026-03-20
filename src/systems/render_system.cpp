//
// Created by jhone on 18/03/2026.
//

#include "systems/render_system.h"
#include <SDL3_image/SDL_image.h>
#include <filesystem>
#include <ranges>
#include "components/components.h"
#include <stdio.h>

void RenderSystem::load_textures() {
    const char *base_path = "assets/";
    namespace fs = std::filesystem;

    for (const auto &entry: fs::recursive_directory_iterator(base_path)) {
        if (!entry.is_regular_file()) continue;

        std::string path = entry.path().string();
        std::string extension = entry.path().extension().string();

        if (extension == ".png" || extension == ".jpg" || extension == ".jpeg") {
            SDL_Surface *surface = IMG_Load(path.c_str());
            if (!surface) {
                SDL_Log("Error loading image %s: %s", path.c_str(), SDL_GetError());
                continue;
            }

            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_DestroySurface(surface);

            if (!texture) {
                SDL_Log("Error creating texture %s: %s", path.c_str(), SDL_GetError());
                continue;
            }

            std::string key = entry.path().string();

            textures[key] = texture;

           // SDL_LogDebug(SDL_LOG_CATEGORY_ERROR,"Loaded texture: %s", key.c_str());
            //printf("Loaded texture: %s\n", key.c_str());

        }
    }
}

RenderSystem::RenderSystem(entt::registry &registry, SDL_Renderer *renderer): System(registry) {
    this->renderer = renderer;
    load_textures();
}

RenderSystem::~RenderSystem() {
    for (const auto &texture: textures | std::views::values) {
        SDL_DestroyTexture(texture);
    }
    textures.clear();
}

void RenderSystem::run(float dt) {
    for (auto view = registry.view<Transform, Sprite>(); const auto entity: view) {
        auto &&[transform, sprite] = view.get<Transform, Sprite>(entity);

        if (textures.contains(sprite.texture_path)) {
            SDL_Texture *texture = textures[sprite.texture_path];
            SDL_FRect dst;
            dst.x = transform.position.x;
            dst.y = transform.position.y;
            float w, h;
            SDL_GetTextureSize(texture, &w, &h);
            dst.w = w * transform.scale.x;
            dst.h = h * transform.scale.y;

            SDL_RenderTexture(renderer, texture, nullptr,&dst );
        }

    }
}
