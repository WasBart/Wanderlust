#version 430 core

in vec3 worldNormal;
in vec2 fragmentUV;
in vec3 fragPos;
in vec3 viewPos;
in vec4 fragPosLightSpace; 

layout (location = 0) out vec4 fragColor;

uniform sampler2D tex;
uniform sampler2D shadowMap;

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


float ShadowCalculation(vec4 fragPosLightSpace)
{
   
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
   
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
   
    float currentDepth = projCoords.z;
   
    vec3 normal = normalize(worldNormal);
    vec3 lightDir = normalize(light.direction - fragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
 
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}


void main()
{
   
   
   //Ambient Part
    vec3 ambient =  mat.ambient * light.ambient;
	  
	//Diffuse Part
	vec3 norm = normalize(worldNormal);
    vec3 lightDir = normalize(light.direction - fragPos);
	float diff = max(dot(lightDir,norm), 0.0);
	vec3 diffuse = diff * mat.diffuse * light.diffuse;
	
    vec3 textureColor = texture(tex, fragmentUV).rgb;
	
	//Specular Part 
	
	vec3 viewDir = normalize(viewPos-fragPos);
	vec3 halfWayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(viewDir, halfWayDir),0.0f), 64);
	vec3 specular = light.specular * spec * mat.specular; 

	//Resulting Light
	
	float shadow = ShadowCalculation(fragPosLightSpace);
	vec3 result = (ambient + (1.0 - shadow) * (diffuse+specular)* textureColor);
	fragColor = vec4(result,1.0);
}