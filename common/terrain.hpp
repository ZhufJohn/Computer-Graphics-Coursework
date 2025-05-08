#pragma once
#include "common.hpp"

class Terrain
{
public:
	Terrain(float heightScale, float blockScale);
	~Terrain();

	bool loadHeightmap(const std::string& filename, unsigned char bitsPerPixel, unsigned int width, unsigned int height);
	
	float getHeightAt(const glm::vec3& position);
	
	void draw(unsigned int& shaderID);

private:
	void generateIndexBuffer();
	void generateNormals();
	void generateVertexBuffers();

	std::streampos getFileLength(std::ifstream& file);
	float getHeightValue(const unsigned char* data, unsigned char numBytes);

	unsigned int loadTexture(const char* path);

private:
	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec2> m_texCoords;
	std::vector<unsigned int> m_indexs;

	glm::uvec2 m_heightmapDimensions;
	float m_heightScale;
	float m_blockScale;

	unsigned int m_VAO, m_VBO, m_EBO;

	unsigned int m_grassTexture;
	unsigned int m_rockTexture;
	unsigned int m_snowTexture;
};
