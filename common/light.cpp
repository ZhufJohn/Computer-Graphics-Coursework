#include "light.hpp"

Light::Light() 
: lightColor(1.0f)
, lightPosition(0.0f)
, ambientIntensity(0.01f)
, diffuseIntensity(1.0f)
{

}

Light::~Light()
{

}

PointLight::PointLight()
	: constantFactor(1.0f)
	, linearFactor(0.09f)
	,  expFactor(0.032f)
{

}

PointLight::~PointLight()
{

}

void PointLight::draw(unsigned int shaderID)
{
	glUniform3f(glGetUniformLocation(shaderID, "lightColor"), lightColor.r, lightColor.g, lightColor.b);
	m_lightSphere.draw(shaderID);
}
