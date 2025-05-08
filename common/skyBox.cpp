#include "skyBox.hpp"

SkyBox::SkyBox()
	:m_skyTexture(-1)
{
	initSkyTexture();
}

SkyBox::~SkyBox()
{

}

void SkyBox::draw(unsigned int shaderID)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyTexture);

	m_sphere.draw(shaderID);
}

void SkyBox::initSkyTexture()
{
	std::vector<std::string> faces;
	faces.push_back("../assets/skybox/right.jpg");
	faces.push_back("../assets/skybox/left.jpg");
	faces.push_back("../assets/skybox/top.jpg");
	faces.push_back("../assets/skybox/bottom.jpg");
	faces.push_back("../assets/skybox/front.jpg");
	faces.push_back("../assets/skybox/back.jpg");

	glGenTextures(1, &m_skyTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyTexture);

	for (int i = 0; i < faces.size(); i++) {
		int width, height, nrComponents;
		unsigned char* data = stbi_load((faces[i]).c_str(), &width, &height, &nrComponents, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else {
			std::cout << "Failed to load texture when create cube map texture: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
