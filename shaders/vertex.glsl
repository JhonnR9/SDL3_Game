#version 450

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;

layout(location = 0) out vec2 v_uv;
layout(location = 1) out vec4 v_color;

layout(std140, set = 1, binding = 0) uniform VertexUniforms
{
    mat4 mvp;
} ubo;

void main()
{
    v_uv = a_uv;
    v_color = a_color;
    gl_Position = ubo.mvp * vec4(a_position, 1.0);
}