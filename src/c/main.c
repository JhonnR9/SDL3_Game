#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cglm/cglm.h>
#include <SDL3_image/SDL_image.h>


typedef struct {
    mat4 mvp;
} UniformData;

typedef struct {
    float x, y, z;
    float u, v;
} Vertex;

typedef struct {
    SDL_Window *window;
    SDL_GPUDevice *device;
    SDL_GPUGraphicsPipeline *pipeline;
    SDL_GPUBuffer *vertex_buffer;
    SDL_GPUBuffer *index_buffer;
    bool supports_mailbox;
    int width, height;
    SDL_GPUTexture *texture;
    SDL_GPUSampler *sampler;
} AppState;


static const Vertex QUAD_VERTICES[] = {
    {-0.5f, 0.5f, 0.0f, 0.0f, 0.0f}, // TL - Top Left
    {0.5f, 0.5f, 0.0f, 1.0f, 0.0f}, // TR - Top Right
    {-0.5f, -0.5f, 0.0f, 0.0f, 1.0f}, // BL - Bottom Left
    {0.5f, -0.5f, 0.0f, 1.0f, 1.0f} // BR - Bottom Right
};

static const unsigned int QUAD_INDICES[] = {
    0, 1, 2, // (TL, TR, BL)
    2, 1, 3 // (BL, TR, BR)
};

static SDL_GPUShader *LoadShader(SDL_GPUDevice *device, const char *path, SDL_GPUShaderStage stage,
                                 uint32_t num_uniforms, uint32_t num_sampler) {
    size_t size;
    void *code = SDL_LoadFile(path, &size);
    if (!code) return NULL;

    SDL_GPUShaderCreateInfo shader_info = {
        .code = code,
        .code_size = size,
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = stage,
        .num_uniform_buffers = num_uniforms,
        .num_samplers = num_sampler

    };

    SDL_GPUShader *shader = SDL_CreateGPUShader(device, &shader_info);
    free(code);
    return shader;
}


static bool CreateVertexBuffer(AppState *app) {
    const uint32_t buffer_size = sizeof(QUAD_VERTICES);

    app->vertex_buffer = SDL_CreateGPUBuffer(app->device, &(SDL_GPUBufferCreateInfo){
                                                 .size = buffer_size,
                                                 .usage = SDL_GPU_BUFFERUSAGE_VERTEX
                                             });

    SDL_GPUTransferBuffer *transfer_buffer = SDL_CreateGPUTransferBuffer(
        app->device, &(SDL_GPUTransferBufferCreateInfo){
            .size = buffer_size,
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD
        });

    // Upload dos dados
    void *map = SDL_MapGPUTransferBuffer(app->device, transfer_buffer, false);
    SDL_memcpy(map, QUAD_VERTICES, buffer_size);
    SDL_UnmapGPUTransferBuffer(app->device, transfer_buffer);

    SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(app->device);
    SDL_GPUCopyPass *copy = SDL_BeginGPUCopyPass(cmd);
    SDL_UploadToGPUBuffer(copy,
                          &(SDL_GPUTransferBufferLocation){.transfer_buffer = transfer_buffer},
                          &(SDL_GPUBufferRegion){.buffer = app->vertex_buffer, .size = buffer_size},
                          false);
    SDL_EndGPUCopyPass(copy);
    SDL_SubmitGPUCommandBuffer(cmd);

    SDL_ReleaseGPUTransferBuffer(app->device, transfer_buffer);

    return app->vertex_buffer != NULL;
}

static bool CreateIndexBuffer(AppState *app) {
    const uint32_t buffer_size = sizeof(QUAD_INDICES);

    // Cria o buffer de índice
    app->index_buffer = SDL_CreateGPUBuffer(app->device, &(SDL_GPUBufferCreateInfo){
                                                .size = buffer_size,
                                                .usage = SDL_GPU_BUFFERUSAGE_INDEX // OBS: uso de índice
                                            });

    SDL_GPUTransferBuffer *transfer_buffer = SDL_CreateGPUTransferBuffer(
        app->device, &(SDL_GPUTransferBufferCreateInfo){
            .size = buffer_size,
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD
        });

    // Upload dos dados
    void *map = SDL_MapGPUTransferBuffer(app->device, transfer_buffer, false);
    SDL_memcpy(map, QUAD_INDICES, buffer_size);
    SDL_UnmapGPUTransferBuffer(app->device, transfer_buffer);

    SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(app->device);
    SDL_GPUCopyPass *copy = SDL_BeginGPUCopyPass(cmd);
    SDL_UploadToGPUBuffer(copy,
                          &(SDL_GPUTransferBufferLocation){.transfer_buffer = transfer_buffer},
                          &(SDL_GPUBufferRegion){.buffer = app->index_buffer, .size = buffer_size},
                          false);
    SDL_EndGPUCopyPass(copy);
    SDL_SubmitGPUCommandBuffer(cmd);

    SDL_ReleaseGPUTransferBuffer(app->device, transfer_buffer);

    return app->index_buffer != NULL;
}

static bool CreatePipeline(AppState *app) {
    SDL_GPUShader *vtx_shader = LoadShader(app->device, "shaders/vertex.spv", SDL_GPU_SHADERSTAGE_VERTEX, 1, 0);
    SDL_GPUShader *frag_shader = LoadShader(app->device, "shaders/fragment.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 1);

    if (!vtx_shader || !frag_shader) return false;

    const SDL_GPUGraphicsPipelineCreateInfo info = {
        .target_info.num_color_targets = 1,
        .target_info.color_target_descriptions = (SDL_GPUColorTargetDescription[]){
            {
                .format = SDL_GetGPUSwapchainTextureFormat(app->device, app->window),
                .blend_state = {
                    .enable_blend = true,
                    .color_blend_op = SDL_GPU_BLENDOP_ADD,
                    .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
                    .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                    .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                    .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                    .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                }
            }
        },
        .vertex_input_state = {
            .num_vertex_buffers = 1,
            .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){
                {
                    .slot = 0, .pitch = sizeof(Vertex), .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX
                }
            },
            .num_vertex_attributes = 2,
            .vertex_attributes = (SDL_GPUVertexAttribute[]){
                {.location = 0, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 0},
                {
                    .location = 1, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                    .offset = sizeof(float) * 3
                }
            }
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_shader = vtx_shader,
        .fragment_shader = frag_shader
    };

    app->pipeline = SDL_CreateGPUGraphicsPipeline(app->device, &info);

    SDL_ReleaseGPUShader(app->device, vtx_shader);
    SDL_ReleaseGPUShader(app->device, frag_shader);

    return app->pipeline != NULL;
}


SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    AppState *app = SDL_malloc(sizeof(AppState));
    *appstate = app;

    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;

    app->window = SDL_CreateWindow("SDL3 GPU Refactored", 640, 480, SDL_WINDOW_RESIZABLE);
    app->device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);

    if (!app->device || !SDL_ClaimWindowForGPUDevice(app->device, app->window)) return SDL_APP_FAILURE;

    app->supports_mailbox = SDL_WindowSupportsGPUPresentMode(app->device, app->window, SDL_GPU_PRESENTMODE_MAILBOX);
    SDL_SetGPUSwapchainParameters(app->device, app->window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
                                  app->supports_mailbox ? SDL_GPU_PRESENTMODE_MAILBOX : SDL_GPU_PRESENTMODE_VSYNC);

    if (!CreateVertexBuffer(app) || !CreatePipeline(app) || !CreateIndexBuffer(app)) return SDL_APP_FAILURE;

    SDL_Surface *surface_texture = IMG_Load("assets/image.png");

    if (surface_texture != NULL) {
        SDL_Surface *rgba = SDL_ConvertSurface(surface_texture, SDL_PIXELFORMAT_RGBA32);
        SDL_DestroySurface(surface_texture);

        SDL_GPUTextureCreateInfo texture_info = {0};
        texture_info.type = SDL_GPU_TEXTURETYPE_2D;
        texture_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        texture_info.width = rgba->w;
        texture_info.height = rgba->h;
        texture_info.layer_count_or_depth = 1;
        texture_info.num_levels = 1;
        texture_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

        app->texture = SDL_CreateGPUTexture(app->device, &texture_info);

        uint32_t size = rgba->w * rgba->h * 4;

        SDL_GPUTransferBuffer *transfer = SDL_CreateGPUTransferBuffer(
            app->device,
            &(SDL_GPUTransferBufferCreateInfo){
                .size = size,
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD
            }
        );

        void *map = SDL_MapGPUTransferBuffer(app->device, transfer, false);
        SDL_memcpy(map, rgba->pixels, size);
        SDL_UnmapGPUTransferBuffer(app->device, transfer);

        SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(app->device);

        SDL_GPUCopyPass *copy = SDL_BeginGPUCopyPass(cmd);

        SDL_UploadToGPUTexture(
            copy,
            &(SDL_GPUTextureTransferInfo){
                .transfer_buffer = transfer,
                .offset = 0,
                .pixels_per_row = rgba->w,
                .rows_per_layer = rgba->h
            },
            &(SDL_GPUTextureRegion){
                .texture = app->texture,
                .w = rgba->w,
                .h = rgba->h,
                .d = 1
            },
            false
        );

        app->sampler = SDL_CreateGPUSampler(
            app->device,
            &(SDL_GPUSamplerCreateInfo){
                .min_filter = SDL_GPU_FILTER_LINEAR,
                .mag_filter = SDL_GPU_FILTER_LINEAR,
                .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
                .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT
            }
        );

        SDL_EndGPUCopyPass(copy);
        SDL_SubmitGPUCommandBuffer(cmd);

        SDL_ReleaseGPUTransferBuffer(app->device, transfer);
        SDL_DestroySurface(rgba);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *app = (AppState *) appstate;
    uint32_t flags = SDL_GetWindowFlags(app->window);

    if (flags & SDL_WINDOW_MINIMIZED) {
        return SDL_APP_CONTINUE;
    }

    SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(app->device);
    if (!cmd) return SDL_APP_CONTINUE;

    SDL_GPUTexture *swap_tex;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, app->window, &swap_tex, (uint32_t *) &app->width,(uint32_t *) &app->height)|| app->width <= 0 || app->height <= 0) {
        SDL_SubmitGPUCommandBuffer(cmd);
        return SDL_APP_CONTINUE;
    }

    UniformData ubo;
    mat4 model, view, proj;

    glm_mat4_identity(model);
    glm_rotate(model, (float) SDL_GetTicks() * 0.001f, (vec3){0, 0, 1});
    glm_translate_make(view, (vec3){0, 0, -2.0f});


    float aspect = (float) app->width / (float) app->height;
    float ortho_size = 1.0f;
    glm_ortho(-ortho_size * aspect, ortho_size * aspect, -ortho_size, ortho_size, 0.1f, 100.0f, proj);

    mat4 view_proj;
    glm_mat4_mul(proj, view, view_proj);
    glm_mat4_mul(view_proj, model, ubo.mvp);


    // Render Pass
    const SDL_GPUColorTargetInfo color_info = {
        .texture = swap_tex,
        .clear_color = {0.04f, 0.06f, 0.07f, 1.0f},
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE
    };

    SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(cmd, &color_info, 1, NULL);
    SDL_BindGPUGraphicsPipeline(pass, app->pipeline);
    SDL_BindGPUVertexBuffers(pass, 0, &(SDL_GPUBufferBinding){.buffer = app->vertex_buffer, .offset = 0}, 1);

    SDL_PushGPUVertexUniformData(cmd, 0, &ubo, sizeof(ubo));

    const vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};

    SDL_PushGPUFragmentUniformData(cmd, 0, color, sizeof(color));

    SDL_GPUTextureSamplerBinding texture_sampler_binding = (SDL_GPUTextureSamplerBinding){
        .texture = app->texture, .sampler = app->sampler
    };

    SDL_BindGPUFragmentSamplers(pass, 0, &texture_sampler_binding, 1);

    SDL_GPUBufferBinding index_buffer_binding = {
        .buffer = app->index_buffer,
        .offset = 0
    };

    SDL_BindGPUIndexBuffer(pass, &index_buffer_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    SDL_DrawGPUIndexedPrimitives(pass, 6, 1, 0, 0, 0);

    SDL_EndGPURenderPass(pass);

    SDL_SubmitGPUCommandBuffer(cmd);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    AppState *app = appstate;

    if (app) {
        if (app->device) {
            SDL_WaitForGPUIdle(app->device);

            if (app->pipeline)
                SDL_ReleaseGPUGraphicsPipeline(app->device, app->pipeline);

            if (app->vertex_buffer)
                SDL_ReleaseGPUBuffer(app->device, app->vertex_buffer);

            if (app->texture)
                SDL_ReleaseGPUTexture(app->device, app->texture);

            if (app->sampler)
                SDL_ReleaseGPUSampler(app->device, app->sampler);
            if (app->index_buffer)
                SDL_ReleaseGPUBuffer(app->device, app->index_buffer);

            SDL_DestroyGPUDevice(app->device);
        }

        if (app->window)
            SDL_DestroyWindow(app->window);

        free(app);
    }

    SDL_Quit();
}
