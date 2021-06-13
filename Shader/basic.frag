#version 430 core

in vec3 worldNormal;
in vec2 fragmentUV;
in vec3 fragPos;
in vec3 viewPos;
in vec4 fragPosLightSpace; 

layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D tex;
layout (binding = 2) uniform sampler2DShadow shadowMap;


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

uniform vec4 singleColor;


float ShadowCalculation(vec4 fragPosLightSpace)
{
   

vec3 sm_ndc = fragPosLightSpace.xyz/ fragPosLightSpace.w;
sm_ndc = sm_ndc * 0.5 + 0.5;
float shadowMult = texture(shadowMap, sm_ndc);

return shadowMult;

   /* vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // [0,1] range
   projCoords = projCoords * 0.5 + 0.5;
   
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
   
    float currentDepth = projCoords.z / fragPosLightSpace.w;
   
    vec3 normal = normalize(worldNormal);
    vec3 lightDir = normalize(light.direction - fragPos);
   
 
    float shadow = (closestDepth > currentDepth ) ? 0.0: 1.0;
	return shadow;*/
}


void main()
{
   
   
   //Ambient Part
    vec3 ambient =  mat.ambient * light.ambient;
	  
	//Diffuse Part
	vec3 norm = normalize(worldNormal);
    vec3 lightDir = normalize(light.direction);
	float diff = max(dot(norm,lightDir), 0.0);
	vec3 diffuse = diff * mat.diffuse * light.diffuse;
	
    vec3 textureColor = texture(tex, fragmentUV).rgb;
	
	//Specular Part 
	
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 halfWayDir = normalize(lightDir + viewDir);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, halfWayDir),0.0f), 10);
	vec3 specular = vec3(1.0, 1.0, 1.0) * spec * vec3(1.0,1.0,1.0); 

	//Resulting Light
	
	float shadow = ShadowCalculation(fragPosLightSpace);
	vec3 result = ((ambient + (1.0 - shadow) * (diffuse+specular)) * textureColor);
	fragColor = mix(vec4(result,1.0),singleColor,singleColor.w);
}