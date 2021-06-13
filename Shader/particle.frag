#version 430

in float fragLifespan;

out vec4 color;

void main()
{
color = vec4(243.0 / 255.0, 243.0 / 255.0, 21.0 / 255.0, fragLifespan / 10);
}