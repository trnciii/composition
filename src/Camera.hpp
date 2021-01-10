#pragma once

#include <glm/glm.hpp>

#include "data.hpp"

struct Camera{
	glm::vec3 pos = glm::vec3(0);
	glm::vec3 basis[3] = {glm::vec3(1,0,0), glm::vec3(0,0,1), glm::vec3(0,-1,0)};
	float flen = 2;

	inline Ray ray(float x, float y) const{
		glm::vec3 dir = basis[0]*x + basis[1]*y - basis[2]*flen;
		return Ray(pos, glm::normalize(dir));
	}

	inline void setDir(glm::vec3 dir, glm::vec3 up){
		basis[0] = glm::normalize(glm::cross(dir, up));
		basis[2] = -glm::normalize(dir);
		basis[1] = glm::cross(basis[2], basis[0]);
	}
};