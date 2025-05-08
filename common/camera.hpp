#pragma once
#include <glm/glm.hpp>
#include "terrain.hpp"

class Camera
{
public:

	Camera(int width, int height);
	~Camera();

	void update(double curFrame, double deltaFrame);

	void onKeyboard(int key);
	void onMouseUp();
	void onMouseDown();
	void onMouseMove(double x, double y);
	void onMouseScroll(double xOffset, double yOffset);

	glm::mat4 getViewTransform();

	glm::vec3 position;
	glm::mat4 viewTransform;
	float* projTransform;
	float fov;
	float aspect;
	glm::vec4 viewportRect;
	float near;
	float far;

	bool useConstraints;

	Terrain* terrain;

private:
	float m_yaw;
	float m_pitch;
	float m_roll;

private:
	float m_mouseX;
	float m_mouseY;
	bool m_mousePress;

	glm::vec3 m_target;
	glm::vec3 m_up;

	bool m_firstMouse;

	float m_mouseSensitivity;
	float m_keySensitivity;
	float m_curFrame;
	float m_deltaFrame;
	float m_heightOffset;
};