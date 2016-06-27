#version 430 core


uniform mat4 projection;
uniform mat4 view;
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 uv;

out vec2 fragmentUV;

void main()
{
	gl_Position =   projection * view * vec4(position, 1.0);  
	fragmentUV = uv;
}
