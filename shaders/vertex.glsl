#version 450

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;

layout(location = 0) out vec2 v_uv;

layout(std140, set = 1, binding = 0) uniform VertexUniforms
{
    mat4 mvp;
} ubo;

void main()
{
    v_uv = a_uv;
    gl_Position = ubo.mvp * vec4(a_position, 1.0);
}