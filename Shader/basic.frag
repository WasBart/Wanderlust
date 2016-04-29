#version 430 core

in vec3 worldNormal;
in vec2 fragmentUV;
in vec3 fragPos;

layout (location = 0) out vec4 fragColor;

uniform sampler2D tex;

struct Light
{
 vec3 position; 
 vec3 direction;
 vec3 ambient;
 vec3 diffuse;
};

uniform Light light; 

void main()
{
   
   
    vec3 ambient =  light.ambient * light.diffuse; 
	  
	vec3 norm = normalize(worldNormal);

    vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.diffuse;
	vec3 textureColor = texture(tex, fragmentUV).rgb;
	vec3 result = (ambient + diffuse) * textureColor;
	fragColor = vec4(result,1.0);
}