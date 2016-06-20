#version 430 core

layout(location = 0) out vec4 color;

in vec2 UV;

layout(binding = 0) uniform sampler2D render_texture;
layout(location = 3) uniform vec2 offset[25];

void main()
{
	vec4 tex_data[25];
	for (int i = 0; i < 25; i++)
	{
		tex_data[i] = texture(render_texture, UV + offset[i]);
	}

	// Apply laplace
	color = 24.0 * tex_data[12];
	for (int i = 0; i < 25; i++)
	{
		if (i != 12)
			color -= tex_data[i];
	}
}