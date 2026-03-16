//
// Created by jhone on 15/03/2026.
//

#ifndef RENDER_2_D_H
#define RENDER_2_D_H

#include <SDL3/SDL.h>
#include <cglm/cglm.h>
#include <array>

typedef struct {
    float x, y, z;
    float u, v;
} Vertex;

typedef struct {
    SDL_Window *window;
    SDL_GPUDevice *device;
} AppContext;

typedef struct {
    mat4 mvp;
} UniformData;

typedef struct {
    SDL_GPUGraphicsPipeline *pipeline;
    SDL_GPUBuffer *vertex_buffer;
    SDL_GPUBuffer *index_buffer;
    uint32_t swap_chain_width;
    uint32_t swap_chain_height;
    SDL_GPUTexture *swap_texture;
    SDL_GPUCommandBuffer *command_buffer;
    SDL_GPURenderPass *render_pass;
} PipelineContext;

typedef struct {
    SDL_GPUTexture *texture;
    SDL_GPUSampler *sampler;
    int width;
    int height;
} Texture2D;

class Renderer2D {
public:
    Renderer2D() = default;
    ~Renderer2D();
    void init(AppContext *ctx);
    Texture2D* load_texture(const char *path);

    void begin_draw();
    void draw_texture(Texture2D* texture);
    void end_draw();

private:
    AppContext *ctx{nullptr};
    PipelineContext* p_ctx{nullptr};

    static SDL_GPUShader *load_shader(
        SDL_GPUDevice *device, const char *path, SDL_GPUShaderStage stage,
        uint32_t num_uniforms, uint32_t num_sampler
    );

    static std::array<Vertex, 4> create_quad_vertex();
    static std::array<unsigned int, 6> create_quad_indices();

    bool create_vertex_buffer();
    bool create_index_buffer();
    bool create_pipeline();


};


#endif //RENDER_2_D_H
