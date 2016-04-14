#version 430 core

in vec3 worldNormal;
in vec2 fragmentUV;

layout (location = 0) out vec4 fragColor;

uniform sampler2D tex;

void main()
{
	vec3 textureColor = texture(tex, fragmentUV).rgb;
	fragColor = vec4(textureColor,1.0);
}