#version 430 core



layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 4) uniform mat4 model; 
layout (location = 5) uniform mat4 view;

uniform mat4 projection;
out vec2 fragmentUV;

void main()
{
	vec4 pos = projection * view * vec4(position, 1.0);
    gl_Position = pos.xyww;
	fragmentUV = uv;
}
