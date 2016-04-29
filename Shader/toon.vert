#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec3 worldNormal;
out vec2 fragmentUV;
out vec3 fragPos;  

layout (location = 4) uniform mat4 model; 
uniform mat4 view;
uniform mat4 projection;

void main()
{
	fragmentUV = uv;
	worldNormal = (view * model * vec4(normal,0)).xyz;
	fragPos =    (model * vec4(position, 1.0f)).xyz;
	gl_Position = projection* view* model* vec4(position,1);
}