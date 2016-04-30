#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec3 worldNormal;
out vec2 fragmentUV;
out vec3 fragPos;  
out vec3 viewPos;

layout (location = 4) uniform mat4 model; 
layout (location = 5) uniform mat4 view;
uniform mat4 projection;

void main()
{
	fragmentUV = uv;
	worldNormal = mat3(transpose(inverse(model))) * normal;  
	fragPos = (model * vec4(position, 1.0f)).xyz;
	viewPos = vec3((inverse(view)[3]));
	gl_Position = projection* view* model* vec4(position,1);
}