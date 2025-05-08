#include "common.hpp"
#include "camera.hpp"
#include "maths.hpp"

Camera::Camera(int width, int height)
	: m_target(0.0f, 0.0f, -1.0f)
	, m_up(0.0f, 1.0f, 0.0f)
	, m_mouseX(0)
	, m_mouseY(0)
	, m_mousePress(false)
	, m_firstMouse(true)
	, position(0.0f, 25, 20)
	, fov(45.0f)
	, near(0.1)
	, far(1000)
	, viewportRect(0.0f, 0.0f, width, height)
	, aspect((float)width / __max(height, 0.1))
	, useConstraints(false)
	, projTransform(nullptr)
	, m_yaw(-90.0f)
	, m_pitch(0.0f)
	, m_roll(0.0f)
	, m_mouseSensitivity(5)
	, m_keySensitivity(10)
	, m_curFrame(0)
	, m_deltaFrame(0)
	, m_heightOffset(5)
{
	projTransform = maths::perspective(fov, aspect, near, far);
	viewTransform = maths::lookAt(position, position + m_target, m_up);
}

Camera::~Camera()
{
	delete projTransform;
}

void Camera::update(double curFrame, double deltaFrame)
{
	m_curFrame = curFrame;
	m_deltaFrame = deltaFrame;

	if (useConstraints)
	{
		float height = terrain->getHeightAt(position);
		if (height > 0)
		{
			position.y = height + m_heightOffset;
		}
	}
}

void Camera::onKeyboard(int key)
{
	float cameraSpeed = m_keySensitivity * m_deltaFrame;

	switch (key)
	{
	case GLFW_KEY_UP:
	case GLFW_KEY_W:
		position = position + cameraSpeed * m_target;
		break;
	case GLFW_KEY_DOWN:
	case GLFW_KEY_S:
		position = position - cameraSpeed * m_target;
		break;
	case GLFW_KEY_LEFT:
	case GLFW_KEY_A:
		position = position - glm::normalize(maths::cross(m_target, m_up)) * cameraSpeed;
		break;
	case GLFW_KEY_RIGHT:
	case GLFW_KEY_D:
		position = position + glm::normalize(maths::cross(m_target, m_up)) * cameraSpeed;
		break;
	default:
		break;
	}
}

void Camera::onMouseUp()
{
	m_mousePress = false;
	m_firstMouse = true;
}

void Camera::onMouseDown()
{
	m_mousePress = true;
}

void Camera::onMouseMove(double x, double y)
{
	if (m_mousePress)
	{
		if (m_firstMouse)
		{
			m_mouseX = x;
			m_mouseY = y;
			m_firstMouse = false;
		}

		float xOffset = x - m_mouseX;
		float yOffset = m_mouseY - y;
		m_mouseX = x;
		m_mouseY = y;

		xOffset *= m_mouseSensitivity * m_deltaFrame;
		yOffset *= m_mouseSensitivity * m_deltaFrame;

		m_yaw += xOffset;
		m_pitch += yOffset;
		if (m_pitch > 89.0f) m_pitch = 89.0f;
		if (m_pitch < -89.0f) m_pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(m_pitch)) * cos(glm::radians(m_yaw));
		front.y = sin(glm::radians(m_pitch));
		front.z = cos(glm::radians(m_pitch)) * sin(glm::radians(m_yaw));
		m_target = glm::normalize(front);
	}
}

void Camera::onMouseScroll(double xOffset, double yOffset)
{
	float cameraSpeed = m_keySensitivity * yOffset * 0.1f;

	position = position + cameraSpeed * m_target;
}

glm::mat4 Camera::getViewTransform()
{
	viewTransform = maths::lookAt(position, position + m_target, m_up);

	return viewTransform;
}
