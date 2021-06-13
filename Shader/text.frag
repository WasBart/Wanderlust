#version 430 core

in vec2 uv;

layout (location = 0) out vec4 color;

uniform sampler2D text;

const vec3 textColor = vec3(0.0, 1.0, 0.0);

void main()
{
    color = vec4(textColor, 1.0) * vec4(1.0, 1.0, 1.0, texture(text, uv).x);
}  