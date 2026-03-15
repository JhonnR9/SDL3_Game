#ifndef LABEL_H
#define LABEL_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct {
    float x;
    float y;
} Vector2;

typedef struct  {
    TTF_Font *font;
    SDL_Texture *font_texture;
    SDL_FRect font_dst;
} Data;

typedef struct {
    const char *text;
    const char *font_file_name;
    float font_size;
    SDL_Color text_color;
    Vector2 position;
    Data data;
} Label;

bool label_init( Label *label , SDL_Renderer *renderer);
void label_renderer(const Label *label, SDL_Renderer *renderer);
Label* label_create(const char *text);


#endif // LABEL_H
