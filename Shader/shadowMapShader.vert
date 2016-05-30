#version 430 core

layout (location = 0) in vec3 position;

layout (location = 1) uniform mat4 lightSpaceMatrix;
layout (location = 4) uniform mat4 model; 
layout (location = 5) uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(position, 1.0f);
}  