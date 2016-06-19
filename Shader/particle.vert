#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in float lifespan;

out float fragLifespan;

layout (location = 3) uniform mat4 mvp;

void main()
{
	fragLifespan = lifespan;
	gl_Position = mvp * vec4(position, 1.0);
	gl_PointSize = 150;
}