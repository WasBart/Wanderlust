#version 430

in vec3 position;
in vec3 normal;
in vec2 uv;

out vec3 worldNormal;
out vec2 fragmentUV;


uniform mat4 model; 
uniform mat4 view;
uniform mat4 projection;

void main()
{
	fragmentUV = uv;
	worldNormal = (model * vec4(normal,0)).xyz;
	gl_Position = projection* view* model* vec4(position,1);
}