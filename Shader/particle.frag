#version 430

in float lifespan; 

out vec4 color;

void main()
{
	color = vec4(243.0 / 255.0, 243.0 / 255.0, 21.0 / 255.0, lifespan / 10);
}