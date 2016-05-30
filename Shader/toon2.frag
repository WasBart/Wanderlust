#version 430 core

in vec3 worldNormal;
in vec2 fragmentUV;
in vec3 fragPos;
in vec3 viewPos;
in vec4 fragPosLightSpace; 

layout (location = 0) out vec4 fragColor;

uniform sampler2D tex;

struct Material{
 vec3 ambient;
 vec3 diffuse; 
 vec3 specular;
 float shininess;
};

struct Light
{
 vec3 position; 
 vec3 direction;
 vec3 ambient;
 vec3 diffuse;
 vec3 specular;
};

uniform Material mat;
uniform Light light; 
const float levels = 3.0;

void main()
{
   
   
   //Ambient Part
    vec3 ambient =  mat.ambient * light.ambient;
	  
	//Diffuse Part
	vec3 norm = normalize(worldNormal);
    vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	diff = floor(diff * levels) / levels;
	vec3 diffuse = diff * light.diffuse;
	
  
	
	//Specular Part 
	
	vec3 viewDir = normalize(viewPos-fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir),0.0f), mat.shininess);
	vec3 specular = light.specular * spec * mat.specular; 

	//Resulting Light
	vec3 result = (ambient + diffuse + specular);
	vec3 textureColor = texture(tex, fragmentUV).rgb;
	
	if(length(textureColor) != 0){
	result = result * textureColor;
	}
	else{
	result = result * mat.diffuse;
	}

	fragColor = vec4(result,1.0);
}