//
// Created by jhone on 15/03/2026.
//

#include "renderer_2_d.h"
#include <SDL3_image/SDL_image.h>

#include "vector_2.h"

SDL_GPUShader *Renderer2D::load_shader(SDL_GPUDevice *device, const char *path, SDL_GPUShaderStage stage,
                                       uint32_t num_uniforms, uint32_t num_sampler) {
    size_t size;
    auto *code = SDL_LoadFile(path, &size);
    if (!code) return nullptr;


    SDL_GPUShaderCreateInfo shader_info = {
        .code = static_cast<const Uint8 *>(code),
        .code_size = size,
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage = stage,
        .num_uniform_buffers = num_uniforms,
        .num_samplers = num_sampler

    };

    SDL_GPUShader *shader = SDL_CreateGPUShader(device, &shader_info);
    SDL_free(code);
    return shader;
}

std::array<Vertex, 4> Renderer2D::create_quad_vertex() {
    return std::array{
        Vertex{0.0f, 0.0f, 0.0f, 0.0f, 0.0f}, // TL
        Vertex{1.0f, 0.0f, 0.0f, 1.0f, 0.0f}, // TR
        Vertex{0.0f, 1.0f, 0.0f, 0.0f, 1.0f}, // BL
        Vertex{1.0f, 1.0f, 0.0f, 1.0f, 1.0f} // BR
    };
}

std::array<unsigned int, 6> Renderer2D::create_quad_indices() {
    return std::array<unsigned int, 6>{
        0, 1, 2, // (TL, TR, BL)
        2, 1, 3 // (BL, TR, BR)
    };
}

bool Renderer2D::create_vertex_buffer() {
    if (!ctx) {
        SDL_Log("App context cannot be null in create vertex buffer");
        return false;
    }

    const auto vertices = create_quad_vertex();
    constexpr auto buffer_size = static_cast<uint32_t>(vertices.size() * sizeof(Vertex));

    SDL_GPUBufferCreateInfo buffer_info = {
        SDL_GPU_BUFFERUSAGE_VERTEX,
        buffer_size
    };

    p_ctx->vertex_buffer = SDL_CreateGPUBuffer(ctx->device, &buffer_info);

    SDL_GPUTransferBufferCreateInfo transfer_buffer_info = {
        SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        buffer_size
    };

    SDL_GPUTransferBuffer *transfer_buffer = SDL_CreateGPUTransferBuffer(
        ctx->device, &transfer_buffer_info);

    void *map = SDL_MapGPUTransferBuffer(ctx->device, transfer_buffer, false);
    SDL_memcpy(map, vertices.data(), buffer_size);
    SDL_UnmapGPUTransferBuffer(ctx->device, transfer_buffer);

    SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(ctx->device);
    SDL_GPUCopyPass *copy = SDL_BeginGPUCopyPass(cmd);

    SDL_GPUTransferBufferLocation transfer_buffer_location = {
        transfer_buffer,

    };

    const SDL_GPUBufferRegion buffer_region = {
        p_ctx->vertex_buffer,
        0,
        buffer_size,
    };

    SDL_UploadToGPUBuffer(copy, &transfer_buffer_location, &buffer_region, false);
    SDL_EndGPUCopyPass(copy);
    SDL_SubmitGPUCommandBuffer(cmd);

    SDL_ReleaseGPUTransferBuffer(ctx->device, transfer_buffer);

    if (!p_ctx->vertex_buffer) {
        SDL_Log("Failed to create vertex buffer");
        return false;
    }

    return true;
}

bool Renderer2D::create_index_buffer() {
    if (!ctx) {
        SDL_Log("App context cannot be null in create index buffer");
        return false;
    }

    const auto indices = create_quad_indices();
    constexpr auto buffer_size = static_cast<uint32_t>(indices.size() * sizeof(uint32_t));

    SDL_GPUBufferCreateInfo buffer_info = {
        SDL_GPU_BUFFERUSAGE_INDEX,
        buffer_size
    };

    p_ctx->index_buffer = SDL_CreateGPUBuffer(ctx->device, &buffer_info);
    SDL_GPUTransferBufferCreateInfo transfer_buffer_info = {
        SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        buffer_size
    };

    SDL_GPUTransferBuffer *transfer_buffer = SDL_CreateGPUTransferBuffer(ctx->device, &transfer_buffer_info);

    void *map = SDL_MapGPUTransferBuffer(ctx->device, transfer_buffer, false);
    SDL_memcpy(map, indices.data(), buffer_size);
    SDL_UnmapGPUTransferBuffer(ctx->device, transfer_buffer);

    SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(ctx->device);
    SDL_GPUCopyPass *copy = SDL_BeginGPUCopyPass(cmd);

    SDL_GPUTransferBufferLocation transfer_buffer_location = {
        transfer_buffer
    };

    SDL_GPUBufferRegion buffer_region = {
        p_ctx->index_buffer,
        0,
        buffer_size,
    };

    SDL_UploadToGPUBuffer(copy, &transfer_buffer_location, &buffer_region, false);
    SDL_EndGPUCopyPass(copy);
    SDL_SubmitGPUCommandBuffer(cmd);

    SDL_ReleaseGPUTransferBuffer(ctx->device, transfer_buffer);

    if (!p_ctx->index_buffer) {
        SDL_Log("Failed to create index buffer");
        return false;
    }

    return true;
}

bool Renderer2D::create_pipeline() {
    if (!ctx) {
        SDL_Log("App context cannot be null in create pipeline");
        return false;
    }

    SDL_GPUShader *vtx_shader = load_shader(ctx->device, "shaders/vertex.spv", SDL_GPU_SHADERSTAGE_VERTEX, 1, 0);
    SDL_GPUShader *frag_shader = load_shader(ctx->device, "shaders/fragment.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 1);

    if (!vtx_shader || !frag_shader) return false;

    SDL_GPUColorTargetDescription color_target = {
        .format = SDL_GetGPUSwapchainTextureFormat(ctx->device, ctx->window),
        .blend_state = {
            .enable_blend = true,
            .color_blend_op = SDL_GPU_BLENDOP_ADD,
            .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
            .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
            .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
            .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        }
    };

    SDL_GPUVertexBufferDescription vertex_buffer_desc = {
        .slot = 0,
        .pitch = sizeof(Vertex),
        .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX
    };

    SDL_GPUVertexAttribute vertex_attributes[] = {
        {.location = 0, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 0},
        {.location = 1, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, .offset = sizeof(float) * 3}
    };

    SDL_GPUGraphicsPipelineCreateInfo pipeline_info = {
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = &color_target
        },
        .vertex_input_state = {
            .num_vertex_buffers = 1,
            .vertex_buffer_descriptions = &vertex_buffer_desc,
            .num_vertex_attributes = 2,
            .vertex_attributes = vertex_attributes
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_shader = vtx_shader,
        .fragment_shader = frag_shader
    };

    p_ctx->pipeline = SDL_CreateGPUGraphicsPipeline(ctx->device, &pipeline_info);

    SDL_ReleaseGPUShader(ctx->device, vtx_shader);
    SDL_ReleaseGPUShader(ctx->device, frag_shader);

    return p_ctx->pipeline != nullptr;
}

Renderer2D::~Renderer2D() {
    if (p_ctx) {
        if (p_ctx->pipeline)
            SDL_ReleaseGPUGraphicsPipeline(ctx->device, p_ctx->pipeline);

        if (p_ctx->vertex_buffer)
            SDL_ReleaseGPUBuffer(ctx->device, p_ctx->vertex_buffer);

        if (p_ctx->index_buffer)
            SDL_ReleaseGPUBuffer(ctx->device, p_ctx->index_buffer);

        delete p_ctx;
        p_ctx = nullptr;
    }
}

void Renderer2D::init(AppContext *ctx) {
    this->ctx = ctx;
    p_ctx = new PipelineContext();
    if (!create_vertex_buffer() || !create_index_buffer() || !create_pipeline()) {
        SDL_Log("Failed to init renderer_2d");
    }
}

Texture2D *Renderer2D::load_texture(const char *path) {
    SDL_Surface *surface = IMG_Load(path);
    if (!surface) {
        SDL_Log("Failed to load texture: %s", path);
        return nullptr;
    }

    Texture2D *tex = create_texture_from_surface(surface);

    SDL_DestroySurface(surface);
    return tex;
}


Texture2D *Renderer2D::create_texture_from_surface(SDL_Surface *surface) {
    SDL_Surface *rgba = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    if (!rgba) {
        SDL_Log("Failed to convert surface to RGBA32");
        return nullptr;
    }

    SDL_GPUTextureCreateInfo texture_info = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .width = static_cast<uint32_t>(rgba->w),
        .height = static_cast<uint32_t>(rgba->h),
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER
    };

    auto *texture_2d = new Texture2D();
    texture_2d->width = static_cast<float>(rgba->w);
    texture_2d->height = static_cast<float>(rgba->h);

    texture_2d->texture = SDL_CreateGPUTexture(ctx->device, &texture_info);
    if (!texture_2d->texture) {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        SDL_DestroySurface(rgba);
        delete texture_2d;
        return nullptr;
    }

    const uint32_t size = rgba->w * rgba->h * 4;

    const SDL_GPUTransferBufferCreateInfo transfer_info = {
        .size = size,
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
    };

    SDL_GPUTransferBuffer *transfer_buffer =
            SDL_CreateGPUTransferBuffer(ctx->device, &transfer_info);

    if (!transfer_buffer) {
        SDL_Log("Failed to create transfer buffer");
        SDL_DestroySurface(rgba);
        SDL_ReleaseGPUTexture(ctx->device, texture_2d->texture);
        delete texture_2d;
        return nullptr;
    }

    void *map = SDL_MapGPUTransferBuffer(ctx->device, transfer_buffer, false);
    SDL_memcpy(map, rgba->pixels, size);
    SDL_UnmapGPUTransferBuffer(ctx->device, transfer_buffer);

    SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(ctx->device);
    SDL_GPUCopyPass *copy = SDL_BeginGPUCopyPass(cmd);

    SDL_GPUTextureTransferInfo transfer = {
        .transfer_buffer = transfer_buffer,
        .offset = 0,
        .pixels_per_row = static_cast<uint32_t>(rgba->w),
        .rows_per_layer = static_cast<uint32_t>(rgba->h)
    };

    const SDL_GPUTextureRegion region = {
        .texture = texture_2d->texture,
        .w = static_cast<uint32_t>(rgba->w),
        .h = static_cast<uint32_t>(rgba->h),
        .d = 1
    };

    SDL_UploadToGPUTexture(copy, &transfer, &region, false);

    SDL_GPUSamplerCreateInfo sampler_info = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT
    };

    texture_2d->sampler = SDL_CreateGPUSampler(ctx->device, &sampler_info);

    SDL_EndGPUCopyPass(copy);
    SDL_SubmitGPUCommandBuffer(cmd);

    SDL_ReleaseGPUTransferBuffer(ctx->device, transfer_buffer);
    SDL_DestroySurface(rgba);

    return texture_2d;
}

void Renderer2D::begin_draw() {
    p_ctx->command_buffer = SDL_AcquireGPUCommandBuffer(ctx->device);
    if (!p_ctx->command_buffer) {
        SDL_Log("Failed to acquire GPU command buffer");
        return;
    }

    if (
        !SDL_WaitAndAcquireGPUSwapchainTexture(
            p_ctx->command_buffer,
            ctx->window,
            &p_ctx->swap_texture,
            &p_ctx->swap_chain_width,
            &p_ctx->swap_chain_height
        ) ||
        p_ctx->swap_chain_width <= 0 ||
        p_ctx->swap_chain_height <= 0
    ) {
        SDL_SubmitGPUCommandBuffer(p_ctx->command_buffer);
        return;
    }

    SDL_GPUColorTargetInfo color_info = {
        .texture = p_ctx->swap_texture,
        .clear_color = {0.04f, 0.06f, 0.07f, 1.0f},
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE
    };

    p_ctx->render_pass = SDL_BeginGPURenderPass(p_ctx->command_buffer, &color_info, 1, nullptr);
    SDL_BindGPUGraphicsPipeline(p_ctx->render_pass, p_ctx->pipeline);

    SDL_GPUBufferBinding vertex_buffer_binding = {
        .buffer = p_ctx->vertex_buffer,
        .offset = 0
    };

    UniformData ubo;
    mat4 model, view, proj;

    glm_mat4_identity(model);
    glm_rotate(model, (float) SDL_GetTicks() * 0.001f, (vec3){0, 0, 1});
    glm_translate_make(view, (vec3){0, 0, -2.0f});


    glm_mat4_mul(proj, view, p_ctx->view_proj);
    glm_mat4_mul(p_ctx->view_proj, model, ubo.mvp);

    SDL_PushGPUVertexUniformData(p_ctx->command_buffer, 0, &ubo, sizeof(ubo));

    constexpr vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};

    SDL_PushGPUFragmentUniformData(p_ctx->command_buffer, 0, color, sizeof(color));

    SDL_BindGPUVertexBuffers(p_ctx->render_pass, 0, &vertex_buffer_binding, 1);

    SDL_GPUBufferBinding index_buffer_binding = {
        .buffer = p_ctx->index_buffer,
        .offset = 0
    };

    SDL_BindGPUIndexBuffer(p_ctx->render_pass, &index_buffer_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    const float left = 0.0f;
    float right = static_cast<float>(p_ctx->swap_chain_width);
    float top = 0.0f;
    float bottom = static_cast<float>(p_ctx->swap_chain_height);
    float near = -1.0f;
    float far = 1.0f;

    glm_ortho(left, right, bottom, top, near, far, p_ctx->view_proj);
}

void Renderer2D::draw_texture(Texture2D *texture, Vector2 position, float rotation, Vector2 scale, Vector2 size) {
    UniformData ubo;

    mat4 model;
    glm_mat4_identity(model);

    glm_translate(model, (vec3){position.x, position.y, 0.0f});

    glm_rotate(model, rotation, (vec3){0, 0, 1});

    glm_scale(model, (vec3){scale.x * size.x, scale.y * size.y, 1.0f});

    glm_mat4_mul(p_ctx->view_proj, model, ubo.mvp);

    float tmp_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    memcpy(ubo.color, tmp_color, sizeof(tmp_color));

    SDL_PushGPUVertexUniformData(p_ctx->command_buffer, 0, &ubo, sizeof(ubo));

    SDL_GPUTextureSamplerBinding binding = {
        .texture = texture->texture,
        .sampler = texture->sampler
    };
    SDL_BindGPUFragmentSamplers(p_ctx->render_pass, 0, &binding, 1);

    SDL_DrawGPUIndexedPrimitives(
        p_ctx->render_pass,
        6,
        1,
        0,
        0,
        0
    );
}

void Renderer2D::end_draw() {
    SDL_EndGPURenderPass(p_ctx->render_pass);
    SDL_SubmitGPUCommandBuffer(p_ctx->command_buffer);
}
