#pragma once
#include "common.hpp"

using namespace glm;
namespace maths
{
	vec3 cross(const vec3& left, const vec3& right);

	mat3 rotate(const float degrees, const vec3& axis);

	mat4 lookAt(const vec3& eye, const vec3& target, const vec3& up);

	float* perspective(float fovy, float aspect, float near, float far);
}
