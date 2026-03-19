

#ifndef COMPONENTS_H
#define COMPONENTS_H
#include <string>

#include "vector_2.h"
#include "SDL3/SDL_surface.h"

typedef struct {
    Vector2 position = Vector2(0, 0);
    Vector2 scale = Vector2(1, 1);
    float rotation = 0.0f;
} Transform;

typedef struct {
    std::string texture_path;
    SDL_Color color = {255, 255, 255};
    bool flipped = false;
    Vector2 origin = Vector2(0, 0);
} Sprite;

#endif //COMPONENTS_H
