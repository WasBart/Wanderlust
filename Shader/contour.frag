#version 430 core

layout(location = 0) out vec4 color;

in vec2 UV;

layout(binding = 0) uniform sampler2D render_texture;
layout(binding = 1) uniform sampler2D depth_texture;
layout(location = 3) uniform vec2 offset[25];

void main()
{
	vec4 tex_data[25];
	float mindepth = 1.0;
	for (int i = 0; i < 25; i++)
	{
		tex_data[i] = texture(render_texture, UV + offset[i]);
		mindepth = min(mindepth, texture(depth_texture, UV + offset[i]).r);
	}

	// Apply laplace
	color = 24.0 * tex_data[12];
	for (int i = 0; i < 25; i++)
	{
		if (i != 12)
			color -= tex_data[i];
	}

	if (length(color) > 0.9)
	{
        color = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        color = vec4(0.0);
    }
	gl_FragDepth = mindepth - 0.0001;
}