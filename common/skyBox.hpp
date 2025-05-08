#pragma once
#include "sphere.hpp"

class SkyBox
{
public:
	SkyBox();
	~SkyBox();

	void draw(unsigned int shaderID);

private:
	void initSkyTexture();

private:
	Sphere m_sphere;
	unsigned int m_skyTexture;
};
