//
// Created by jhone on 12/03/2026.
//

#include "label.h"
#include <SDL3_ttf/SDL_ttf.h>
#include "renderer_2_d.h"

Label::Label(const char *text, Renderer2D *renderer) {
    this->renderer = renderer;
    this->data.font_texture = nullptr;
    this->data.font = nullptr;

    this->text = text;
    text_color = SDL_Color{255, 255, 255, 255};
    font_file_name = "assets/04b_03__.TTF";
    font_size = 16;
    position = Vector2{0, 0};

    this->data.font = TTF_OpenFont(this->font_file_name, this->font_size);

    if (!this->data.font) {
        SDL_Log("Couldn't open font %s" , this->font_file_name);
        return;
    }

    update_texture();

}

void Label::update_texture() {
    SDL_Surface *text_surface = TTF_RenderText_Solid(
        this->data.font,
        text,
        0,
        this->text_color
    );

    if (!text_surface) {
        SDL_Log("Failed to render text: %s", SDL_GetError());
        return;
    }

    this->data.font_dst = SDL_FRect{
        this->position.x,
        this->position.y,
        (float)text_surface->w,
        (float)text_surface->h
    };

    this->data.font_texture = renderer->create_texture_from_surface(text_surface);

    SDL_DestroySurface(text_surface);
}

