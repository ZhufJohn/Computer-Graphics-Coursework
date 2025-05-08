#pragma once
#include "common.hpp"

class Sphere
{
public:
	Sphere();
	~Sphere();

public:
	void draw(unsigned int shaderID);

	void initTextures(const char* diffusePath, const char* specularPath, const char* normalPath);
	void drawPhong(unsigned int shaderID);

private:
	void initRenderData();

	unsigned int loadTexture(const char* path);

private:
	glm::vec3 m_color;
	unsigned int m_VBO, m_VAO, m_EBO;
	std::vector<Vertex> m_Vertices;
	std::vector<unsigned int> m_indices;

	unsigned int m_diffuseTexture;
	unsigned int m_specularTexture;
	unsigned int m_normalTexture;
};
