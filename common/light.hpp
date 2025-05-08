#pragma once
#include "common.hpp"
#include "sphere.hpp"

class Light
{
public:
	Light();
	~Light();

public:
	glm::vec3 lightColor;
	glm::vec3 lightPosition;

	float ambientIntensity;
	float diffuseIntensity;
};

class PointLight : public Light
{
public:
	PointLight();
	~PointLight();

	void draw(unsigned int shaderID);

public:
	float constantFactor;
	float linearFactor;
	float expFactor;

private:
	Sphere m_lightSphere;
};
