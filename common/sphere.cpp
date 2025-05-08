#include "sphere.hpp"
#include "maths.hpp"

Sphere::Sphere()
	: m_color(1.0f)
	, m_VBO(-1)
	, m_VAO(-1)
	, m_EBO(-1)
	, m_diffuseTexture(-1)
	, m_specularTexture(-1)
	, m_normalTexture(-1)
{
	initRenderData();
}

Sphere::~Sphere()
{

}

void Sphere::draw(unsigned int shaderID)
{
	glUniform3f(glGetUniformLocation(shaderID, "color"), m_color.r, m_color.g, m_color.b);

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLE_STRIP, m_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Sphere::initTextures(const char* diffusePath, const char* specularPath, const char* normalPath)
{
	m_diffuseTexture = loadTexture(diffusePath);
	m_specularTexture = loadTexture(specularPath);
	m_normalTexture = loadTexture(normalPath);
}

void Sphere::drawPhong(unsigned int shaderID)
{
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shaderID, "diffuseMap"), 0);
	glBindTexture(GL_TEXTURE_2D, m_diffuseTexture);
	glActiveTexture(GL_TEXTURE1);
	glUniform1i(glGetUniformLocation(shaderID, "specularMap"), 1);
	glBindTexture(GL_TEXTURE_2D, m_specularTexture);
	glActiveTexture(GL_TEXTURE2);
	glUniform1i(glGetUniformLocation(shaderID, "normalMap"), 2);
	glBindTexture(GL_TEXTURE_2D, m_normalTexture);

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLE_STRIP, m_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Sphere::initRenderData()
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	//生成顶点数据
	const unsigned int X_SEGMENTS = 64;
	const unsigned int Y_SEGMENTS = 64;
	const float PI = 3.14159265359;
	for (unsigned int y = 0; y <= Y_SEGMENTS; y++) {
		for (unsigned int x = 0; x <= X_SEGMENTS; x++) {
			float xSegment = (float)x / (float)X_SEGMENTS;
			float ySegment = (float)y / (float)Y_SEGMENTS;
			//float xPos = std::cos(xSegment * 2.0f * PI) * std::cos(ySegment * PI);
			//float yPos = std::sin(xSegment * 2.0f * PI);
			//float zPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

			float xPos = std::sin(ySegment * PI) * std::cos(xSegment * 2.0f * PI);
			float yPos = std::cos(ySegment * PI);
			float zPos = std::sin(ySegment * PI) * std::sin(xSegment * 2.0f * PI);

			Vertex v;
			v.position = glm::vec3(xPos, yPos, zPos);
			v.normal = glm::vec3(xPos, yPos, zPos);
			v.texCoord = glm::vec2(xSegment * 5, ySegment * 5);
			v.tangent = glm::normalize(glm::vec3(-zPos, 0, -xPos));
			v.bitangent = glm::normalize(maths::cross(v.normal, v.tangent));
			m_Vertices.push_back(v);
		}
	}

	bool oddRow = false;
	for (int y = 0; y < Y_SEGMENTS; y++) {
		if (!oddRow) {
			for (int x = 0; x <= X_SEGMENTS; x++) {
				m_indices.push_back(y * (X_SEGMENTS + 1) + x);
				m_indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
			}
		}
		else {
			for (int x = X_SEGMENTS; x >= 0; x--) {
				m_indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				m_indices.push_back(y * (X_SEGMENTS + 1) + x);
			}
		}
		oddRow = !oddRow;
	}

	glBindVertexArray(m_VAO);

	//顶点缓冲
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), &m_Vertices[0], GL_STATIC_DRAW);

	//索引缓冲
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

	//位置
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	//法线
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	//纹理
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	//切线
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
	//副切线
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

	glBindVertexArray(0);

}

unsigned int Sphere::loadTexture(const char* path)
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
