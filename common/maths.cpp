#include "maths.hpp"

namespace maths
{

	vec3 cross(const vec3& left, const vec3& right)
	{
		// use matrix * vector form
		mat3 crossMatrix = mat3(
			vec3(0, left.z, -left.y),
			vec3(-left.z, 0, left.x),
			vec3(left.y, -left.x, 0)
		);

		return crossMatrix * right;
	}

	mat3 rotate(const float degrees, const vec3& axis)
	{
		float radians = glm::radians(degrees);
		float cosTheta = cos(radians);
		float sinTheta = sin(radians);

		mat3 identity = mat3(1.0);

		vec3 normAxis = normalize(axis);

		mat3 rot = mat3(
			vec3(normAxis.x * normAxis.x * (1.0f - cosTheta) + cosTheta, normAxis.x * normAxis.y * (1.0f - cosTheta) + normAxis.z * sinTheta, normAxis.x * normAxis.z * (1.0f - cosTheta) - normAxis.y * sinTheta),
			vec3(normAxis.x * normAxis.y * (1.0f - cosTheta) - normAxis.z * sinTheta, normAxis.y * normAxis.y * (1.0f - cosTheta) + cosTheta, normAxis.y * normAxis.z * (1.0f - cosTheta) + normAxis.x * sinTheta),
			vec3(normAxis.x * normAxis.z * (1.0f - cosTheta) + normAxis.y * sinTheta, normAxis.y * normAxis.z * (1.0f - cosTheta) - normAxis.x * sinTheta, normAxis.z * normAxis.z * (1.0f - cosTheta) + cosTheta)
		);

		return rot;
	}

	mat4 lookAt(const vec3& eye, const vec3& target, const vec3& up)
	{
		vec3 forward = normalize(target - eye); // Compute the forward direction (camera looking towards origin)
		vec3 right = normalize(maths::cross(forward, up)); // Compute the right vector using cross product
		vec3 newUp = maths::cross(right, forward); // Compute the corrected up vector

		// Construct the LookAt view matrix
		mat4 view = mat4(
			vec4(right.x, newUp.x, -forward.x, 0.0f),
			vec4(right.y, newUp.y, -forward.y, 0.0f),
			vec4(right.z, newUp.z, -forward.z, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);

		view = translate(view, -eye); // Apply translation to position the camera at `eye`
		return view; // Return the computed view matrix
	}

	float* perspective(float fovy, float aspect, float near, float far)
	{
		// Allocate memory for a 4x4 matrix (16 floats)
		float* matrix = new float[16];

		// Calculate the focal length (cotangent of half the fovy)
		float f = 1.0f / std::tan(fovy / 2.0f);

		// Initialize all matrix elements to 0
		for (int i = 0; i < 16; ++i) {
			matrix[i] = 0.0f;
		}

		// Set the perspective projection values
		matrix[0] = f / aspect;    // (0,0) scale x
		matrix[5] = f;              // (1,1) scale y
		matrix[10] = (far + near) / (near - far);       // (2,2) depth scaling
		matrix[11] = -1.0f;                            // (3,2) set w to -z
		matrix[14] = (2.0f * far * near) / (near - far); // (2,3) depth translation

		// Note: Matrix is stored in column-major order:
		// 0  4  8  12
		// 1  5  9  13
		// 2  6  10 14
		// 3  7  11 15
		return matrix;
	}

}
