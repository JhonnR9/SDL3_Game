#version 450

layout(set = 2, binding = 0) uniform sampler2D texSampler;

layout(set = 3, binding = 0) uniform ColorUniform
{
    vec4 color;
} u_color;

layout(location = 0) in vec2 v_uv;
layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = texture(texSampler, v_uv) * u_color.color;
}