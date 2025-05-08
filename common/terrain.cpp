
#include "terrain.hpp"
#include "maths.hpp"

Terrain::Terrain(float heightScale, float blockScale)
	: m_heightScale(heightScale)
	,m_blockScale(blockScale)
{
	m_grassTexture = loadTexture("../assets/textures/grass.jpg");
	m_rockTexture = loadTexture("../assets/textures/rock.jpg");
	m_snowTexture = loadTexture("../assets/textures/snow.jpg");

	loadHeightmap("../assets/terrain/terrain0-16bbp-257x257.raw", 16, 257, 257);
}

Terrain::~Terrain()
{

}

bool Terrain::loadHeightmap(const std::string& filename, unsigned char bitsPerPixel, unsigned int width, unsigned int height)
{
	std::ifstream ifs;
	ifs.open(filename, std::ifstream::binary);
	if (ifs.fail()) {
		std::cout << "Failed to open file: " << filename << std::endl;
		return false;
	}

	const unsigned int bytesPerPixel = bitsPerPixel / 8;
	const unsigned int expectedFileSize = (bytesPerPixel * width * height);
	std::streampos fileSize = getFileLength(ifs);
	if (expectedFileSize != fileSize) {
		std::cout << "File Size Error: " << std::endl;
		return false;
	}

	unsigned char* heightMap = new unsigned char[fileSize];
	ifs.read((char*)heightMap, fileSize);
	if (ifs.fail()) {
		std::cout << "Error occurred when read height map file: " << filename << std::endl;
		ifs.close();
		delete[] heightMap;
		return false;
	}
	ifs.close();

	unsigned int numVerts = width * height;
	m_positions.resize(numVerts);
	m_normals.resize(numVerts);
	m_texCoords.resize(numVerts);

	m_heightmapDimensions = glm::uvec2(width, height);

	//世界单位
	float terrainWidth = (width - 1) * m_blockScale;
	float terrainHeight = (height - 1) * m_blockScale;

	float halfTerrainWidth = terrainWidth * 0.5f;
	float halfTerrainHeight = terrainHeight * 0.5f;

	for (unsigned int j = 0; j < height; j++) {
		for (unsigned int i = 0; i < width; i++) {
			unsigned int index = (j * width) + i;
			assert(index * bytesPerPixel < fileSize);
			float heightValue = getHeightValue(&heightMap[index * bytesPerPixel], bytesPerPixel);

			float S = (i / (float)(width - 1));
			float T = (j / (float)(height - 1));

			float X = (S * terrainWidth) - halfTerrainWidth;
			float Y = heightValue * m_heightScale;
			float Z = (T * terrainHeight) - halfTerrainHeight;

			m_normals[index] = glm::vec3(0.0f);
			m_positions[index] = glm::vec3(X, Y, Z);
			m_texCoords[index] = glm::vec2(S, T);
		}
	}

	std::cout << "Terrain loaded!" << std::endl;
	delete[] heightMap;

	generateIndexBuffer();
	generateNormals();
	generateVertexBuffers();

	return true;
}

float Terrain::getHeightAt(const glm::vec3& position)
{
	float height = -FLT_MAX;
	if (m_heightmapDimensions.x < 2 || m_heightmapDimensions.y < 2) {
		return height;
	}

	float terrainWidth = (m_heightmapDimensions.x - 1) * m_blockScale;
	float terrainHeight = (m_heightmapDimensions.y - 1) * m_blockScale;
	float halfWidth = terrainWidth * 0.5f;
	float halfHeight = terrainHeight * 0.5f;

	glm::vec3 terrainPos = position;
	glm::vec3 invBlockScale(1.0f / m_blockScale, 0.0f, 1.0f / m_blockScale);

	glm::vec3 offset(halfWidth, 0.0f, halfHeight);

	glm::vec3 vertexIndices = (terrainPos + offset) * invBlockScale;

	int u0 = (int)floorf(vertexIndices.x);
	int u1 = u0 + 1;
	int v0 = (int)floorf(vertexIndices.z);
	int v1 = v0 + 1;

	if (u0 >= 0 && u1 < (int)m_heightmapDimensions.x && v0 >= 0 && v1 < (int)m_heightmapDimensions.y) {
		glm::vec3 p00 = m_positions[(v0 * m_heightmapDimensions.x) + u0];
		glm::vec3 p10 = m_positions[(v0 * m_heightmapDimensions.x) + u1];
		glm::vec3 p01 = m_positions[(v1 * m_heightmapDimensions.x) + u0];
		glm::vec3 p11 = m_positions[(v1 * m_heightmapDimensions.x) + u1];

		float percentU = vertexIndices.x - u0;
		float percentV = vertexIndices.z - v0;

		glm::vec3 dU, dV;
		if (percentU > percentV) {
			dU = p10 - p00;
			dV = p11 - p10;
		}
		else {
			dU = p11 - p01;
			dV = p01 - p00;
		}

		glm::vec3 heightPos = p00 + (dU * percentU) + (dV * percentV);
		height = heightPos.y;
	}

	return height;

}

void Terrain::draw(unsigned int& shaderID)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_grassTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_rockTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_snowTexture);
	glUniform1f(glGetUniformLocation(shaderID, "heightThreshold"), m_heightScale);

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_indexs.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Terrain::generateIndexBuffer()
{
	if (m_heightmapDimensions.x < 2 || m_heightmapDimensions.y < 2) {
		return;
	}

	const unsigned int terrainWidth = m_heightmapDimensions.x;
	const unsigned int terrainHeight = m_heightmapDimensions.y;

	const unsigned int numTriangles = (terrainWidth - 1) * (terrainHeight - 1) * 2;
	m_indexs.resize(numTriangles * 3);

	unsigned int index = 0;
	for (unsigned int j = 0; j < (terrainHeight - 1); j++) {
		for (unsigned int i = 0; i < (terrainWidth - 1); i++) {
			int vertexIndex = (j * terrainWidth) + i;
			//TO
			m_indexs[index++] = vertexIndex;
			m_indexs[index++] = vertexIndex + terrainWidth + 1;
			m_indexs[index++] = vertexIndex + 1;
			//T1
			m_indexs[index++] = vertexIndex;
			m_indexs[index++] = vertexIndex + terrainWidth;
			m_indexs[index++] = vertexIndex + terrainWidth + 1;
		}
	}
}

void Terrain::generateNormals()
{
	for (unsigned int i = 0; i < m_indexs.size(); i += 3) {
		glm::vec3 v0 = m_positions[m_indexs[i + 0]];
		glm::vec3 v1 = m_positions[m_indexs[i + 1]];
		glm::vec3 v2 = m_positions[m_indexs[i + 2]];

		glm::vec3 normal = glm::normalize(maths::cross(v1 - v0, v2 - v0));

		m_normals[m_indexs[i + 0]] += normal;
		m_normals[m_indexs[i + 1]] += normal;
		m_normals[m_indexs[i + 2]] += normal;
	}

	const glm::vec3 up(0.0f, 1.0f, 0.0f);
	for (unsigned int i = 0; i < m_normals.size(); i++) {
		m_normals[i] = glm::normalize(m_normals[i]);
	}
}

void Terrain::generateVertexBuffers()
{
	size_t posSize = m_positions.size() * sizeof(glm::vec3);
	size_t normalSize = m_normals.size() * sizeof(glm::vec3);
	size_t uvSize = m_texCoords.size() * sizeof(glm::vec2);

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, posSize + normalSize + uvSize, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, posSize, &m_positions[0]);
	glBufferSubData(GL_ARRAY_BUFFER, posSize, normalSize, &m_normals[0]);
	glBufferSubData(GL_ARRAY_BUFFER, posSize + normalSize, uvSize, &m_texCoords[0]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)posSize);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)(posSize + normalSize));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexs.size() * sizeof(unsigned int), &m_indexs[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

std::streampos Terrain::getFileLength(std::ifstream& file)
{
	std::streampos pos = file.tellg();
	file.seekg(0, std::ios::end);
	std::streampos length = file.tellg();
	file.seekg(pos);

	return length;
}

float Terrain::getHeightValue(const unsigned char* data, unsigned char numBytes)
{
	switch (numBytes)
	{
	case 1:
	{
		return (unsigned char)(data[0]) / (float)0xff;
	}
	break;
	case 2:
	{
		return (unsigned short)(data[1] << 8 | data[0]) / (float)0xffff;
	}
	break;
	case 4:
	{
		return (unsigned int)(data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0]) / (float)0xffffffff;
	}
	break;
	default:
		assert(false);
		break;
	}

	return 0.0;
}

unsigned int Terrain::loadTexture(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, numComponents;
	unsigned char* data = stbi_load(path, &width, &height, &numComponents, 0);
	if (data)
	{
		GLenum format;
		if (numComponents == 1)
			format = GL_RED;
		else if (numComponents == 3)
			format = GL_RGB;
		else if (numComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture " << path << " failed to load." << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
