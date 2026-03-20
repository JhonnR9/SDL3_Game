//
// Created by jhone on 18/03/2026.
//

#include "systems/render_system.h"
#include <SDL3_image/SDL_image.h>
#include <filesystem>
#include <ranges>
#include "components/components.h"

#include "utils/asset_manager.h"



RenderSystem::RenderSystem(entt::registry &registry, SDL_Renderer *renderer): System(registry) {
    this->renderer = renderer;
    asset_manager = registry.ctx().get<AssetManager*>();

    if (asset_manager) {
        asset_manager->load_all_from_folder("assets/atlas_texture");
    }

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

        if (!asset_manager)return;

        if (SDL_Texture *texture = asset_manager->get_texture(sprite.texture_path)) {
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
