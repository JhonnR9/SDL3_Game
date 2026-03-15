//
// Created by jhone on 12/03/2026.
//

#include "label.h"
#include <SDL3_ttf/SDL_ttf.h>

bool label_init(Label *label, SDL_Renderer *renderer) {
    label->data.font_texture = NULL;
    label->data.font = NULL;

    label->data.font = TTF_OpenFont(label->font_file_name, label->font_size);

    if (!label->data.font) {
        SDL_Log("Couldn't open font: %s", SDL_GetError());
        return false;
    }

    const char* final_text = (label->text && label->text[0] != '\0') ? label->text : "empty text";

    SDL_Surface *text = TTF_RenderText_Solid(label->data.font, final_text, 0, label->text_color);

    label->data.font_dst = (SDL_FRect){
        label->position.x,
        label->position.y,
        (float) text->w,
        (float) text->h
    };

    label->data.font_texture = SDL_CreateTextureFromSurface(renderer, text);
    SDL_DestroySurface(text);
    return true;
}

void label_renderer(const Label *label, SDL_Renderer *renderer) {
    SDL_RenderTexture(renderer, label->data.font_texture, NULL, &label->data.font_dst);
}

Label * label_create(const char *text) {
    Label *label = malloc(sizeof(Label));
    label->text = text;
    label->text_color = (SDL_Color){255, 255, 255};
    label->font_file_name = "assets/04b_03__.ttf";
    label->font_size = 16;
    label->position = (Vector2){0, 0};
    return label;
}
