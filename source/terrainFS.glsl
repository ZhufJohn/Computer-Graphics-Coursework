#version 330 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 fragPos;
in vec3 localNormal;

uniform sampler2D texture_grass;
uniform sampler2D texture_rock;
uniform sampler2D texture_snow;
uniform float heightThreshold;//阈值之上用snow 之下根据陡峭程度混合grass和rock

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

vec3 calcLightCommon(vec3 color, float ambientIntensity, float diffuseIntensity, vec3 lightDirection, vec3 normal, vec3 viewDir, vec3 terrainColor){
	//diffuse贴图颜色
	vec3 diffuseTex = terrainColor;
	//光源方向
	vec3 lightDir = lightDirection;
	//环境光
	vec3 ambient = color * diffuseTex * ambientIntensity;
	//漫反射
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = color * diffuseIntensity * diffuseTex * diff;
	
	return (ambient + diffuse);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 terrainColor)
{
	vec3 lightDirection = light.position - fragPos;
	float distance = length(lightDirection);
	
	vec3 result = calcLightCommon(light.color, light.ambientIntensity, light.diffuseIntensity, normalize(lightDirection), normal, viewDir, terrainColor);
	float attenuation = light.constant + 
						light.linear * distance +
						light.exp * distance * distance;
	return result / attenuation;
}

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 terrainColor){	
	return calcLightCommon(light.color, light.ambientIntensity, light.diffuseIntensity, normalize(light.direction), normal, viewDir, terrainColor);
}

void main()
{
	vec3 grassColor = texture(texture_grass, texCoord * 16.0).rgb;
	vec3 rockColor = texture(texture_rock, texCoord * 32.0).rgb;
	vec3 snowColor = texture(texture_snow, texCoord * 32.0).rgb;

	vec3 normal = normalize(localNormal);
	float factor = dot(normal, vec3(0.0, 1.0, 0.0));
	vec3 mixColor = mix(grassColor, rockColor, 1 - factor);

	factor = fragPos.y / heightThreshold;
	factor = pow(factor, 4);
	mixColor = mix(snowColor, mixColor, 1 - factor);

	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 result = vec3(0.0);
	result += calcDirLight(dirLight, normal, viewDir, mixColor);
	for (int i=0; i<1; i++)
	{
		result += calcPointLight(pointLights[i], normal, viewDir, mixColor);
	}

	FragColor = vec4(result, 1.0);
};