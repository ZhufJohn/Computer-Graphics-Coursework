#version 330 core

out vec4 fragColor;

in vec3 fragPos;
in vec2 texCoord;
in mat3 TBN;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

uniform vec3 viewPos;

struct PointLight
{
	vec3 color;
	vec3 position;
	float ambientIntensity;
	float diffuseIntensity; 
	float constant;
	float linear;
	float exp;
};
uniform PointLight pointLights[1];

struct DirLight{
	vec3 color;
	float ambientIntensity;
	float diffuseIntensity; 
	vec3 direction;
};
uniform DirLight dirLight;

vec3 calcLightCommon(vec3 color, float ambientIntensity, float diffuseIntensity, vec3 lightDirection, vec3 normal, vec3 viewDir){
	//diffuse贴图颜色
	vec3 diffuseTex = texture(diffuseMap, vec2(texCoord.x, 1.0 - texCoord.y)).rgb;
	//specular贴图颜色
	vec3 specularTex = texture(specularMap, vec2(texCoord.x, 1.0 - texCoord.y)).rgb;
	//光源方向
	vec3 lightDir = lightDirection;
	//环境光
	vec3 ambient = color * diffuseTex * ambientIntensity;
	//漫反射
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = color * diffuseIntensity * diffuseTex * diff;
	//镜面反射
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
	vec3 specular = color * specularTex * spec;
	
	return (ambient + diffuse + specular);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDirection = light.position - fragPos;
	float distance = length(lightDirection);
	
	vec3 result = calcLightCommon(light.color, light.ambientIntensity, light.diffuseIntensity, normalize(lightDirection), normal, viewDir);
	float attenuation = light.constant + 
						light.linear * distance +
						light.exp * distance * distance;
	return result / attenuation;
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir){	
	return calcLightCommon(light.color, light.ambientIntensity, light.diffuseIntensity, normalize(light.direction), normal, viewDir);
}

void main (void) 
{
	//用TBN矩阵计算法向量
	vec3 normal = texture(normalMap, vec2(texCoord.x, 1.0 - texCoord.y)).rgb;
	normal = normalize(normal * 2.0 - 1.0);//to [-1,1]
	normal = normalize(TBN * normal);

	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 result = vec3(0.0);
	result += calcDirLight(dirLight, normal, viewDir);
	for (int i=0; i<1; i++)
	{
		result += calcPointLight(pointLights[i], normal, viewDir);
	}

	fragColor = vec4(result, 1.0);
}