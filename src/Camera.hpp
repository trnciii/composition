#pragma once

#include <glm/glm.hpp>

#include "data.hpp"

struct Camera{
	glm::vec3 position = glm::vec3(0);
	glm::mat3 toWorld = glm::mat3{	1, 0, 0,
									0, 0, 1,
									0,-1, 0};
	float flen = 2;

	inline Ray ray(float x, float y) const{
		return Ray(position, glm::normalize(toWorld*glm::vec3(x, y, -flen)));
	}

	inline void setDir(glm::vec3 dir, glm::vec3 up){
		glm::vec3 b0 = glm::normalize(glm::cross(dir, up));
		glm::vec3 b2 = -glm::normalize(dir);
		glm::vec3 b1 = glm::cross(b2, b0);

		toWorld = glm::mat3(b0, b1, b2);
	}
};