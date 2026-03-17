#ifndef LABEL_H
#define LABEL_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "renderer_2_d.h"
#include "vector_2.h"



class Label {
public:
    struct Data {
        TTF_Font* font = nullptr;
        Texture2D* font_texture = nullptr;
        SDL_FRect font_dst{};
    };

    Label(const char* text, Renderer2D* renderer);
    void update_texture();


private:
    Renderer2D* renderer;
    Data data{};

public:
    Data get_data() const {
        return data;
    }

    const char * get_text() const {
        return text;
    }

    void set_text(const char *text) {
        this->text = text;
    }

    const char * get_font_file_name() const {
        return font_file_name;
    }

    void set_font_file_name(const char *font_file_name) {
        this->font_file_name = font_file_name;
        update_texture();
    }

    float get_font_size() const {
        return font_size;
    }

    void set_font_size(float font_size) {
        this->font_size = font_size;

        this->data.font = TTF_OpenFont(this->font_file_name, this->font_size);

        if (!this->data.font) {
            SDL_Log("Couldn't open font %s" , this->font_file_name);
            return;
        }
        update_texture();
    }

    SDL_Color get_text_color() const {
        return text_color;
    }

    void set_text_color(const SDL_Color &text_color) {
        update_texture();
        this->text_color = text_color;
    }

    Vector2 get_position() const {
        return position;
    }

    void set_position(const Vector2 &position) {
        this->position = position;
    }

private:
    const char* text = nullptr;
    const char* font_file_name = nullptr;
    float font_size = 0.0f;
    SDL_Color text_color{};
    Vector2 position{};

};

#endif // LABEL_H
