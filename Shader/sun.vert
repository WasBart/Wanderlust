#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec3 worldNormal;
out vec2 fragmentUV;


layout (location = 4) uniform mat4 model; 
layout (location = 5) niform mat4 view;
uniform mat4 projection;

void main()
{
	fragmentUV = uv;
	worldNormal = (model * vec4(normal,0)).xyz;
	gl_Position = projection* view* model* vec4(position,1);
}