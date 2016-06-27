#version 430 core

in vec2 fragmentUV;

layout (binding = 0) uniform sampler2D tex;

out vec4 fragColor;

void main(){
	fragColor = vec4(texture(tex, fragmentUV),1.0);
}