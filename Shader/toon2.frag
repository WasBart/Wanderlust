#version 430 core

in vec3 worldNormal;
in vec2 fragmentUV;
in vec3 fragPos;
in vec3 viewPos;
in vec4 fragPosLightSpace; 

layout (location = 0) out vec4 fragColor;


layout (binding = 0) uniform sampler2D tex;
layout (binding = 2) uniform sampler2DShadow shadowMap;
layout (depth_greater) out float gl_FragDepth;

uniform vec4 singleColor;

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

float ShadowCalculation(vec4 fragPosLightSpace)
{
   

vec3 sm_ndc = fragPosLightSpace.xyz/ fragPosLightSpace.w;
sm_ndc = sm_ndc * 0.5 + 0.5;
float shadowMult = texture(shadowMap, sm_ndc);

return shadowMult;

}


void main()
{
   
   
   //Ambient Part
    vec3 ambient =  mat.ambient * light.ambient;
	  
	//Diffuse Part
	vec3 norm = normalize(worldNormal);
    vec3 lightDir = normalize(light.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	diff = floor(diff * levels)/ levels;
	vec3 diffuse = diff * light.diffuse;
	
  
	
	//Specular Part 
	
	vec3 viewDir = normalize(viewPos);
	vec3 reflectDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(viewDir, reflectDir),0.0f), mat.shininess);
	vec3 specular = light.specular * spec * mat.specular; 

	//Resulting Light
	float shadow = ShadowCalculation(fragPosLightSpace);
	vec3 result = ambient + (1.0 - shadow) * (diffuse+specular);
	vec3 textureColor = texture(tex, fragmentUV).rgb;
	
	if(length(textureColor) != 0){
	result = result * textureColor;
	}
	else{
	result = result * mat.diffuse;
	}
	gl_FragDepth = mix(1.0, gl_FragCoord.z, texture(tex, fragmentUV).w);
	fragColor = mix(vec4(result, texture(tex,fragmentUV).w) , singleColor ,singleColor.w);
}