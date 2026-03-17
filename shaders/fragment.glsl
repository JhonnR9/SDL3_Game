#version 450

layout(set = 2, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 v_uv;
layout(location = 1) in vec4 v_color;
layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = texture(texSampler, v_uv) * v_color;
}