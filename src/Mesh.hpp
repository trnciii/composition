#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Vertex{
	glm::vec3 position;
	glm::vec3 normal;
};

struct Index{
	uint32_t v0;
	uint32_t v1;
	uint32_t v2;

	uint32_t mtlID;
};

struct Mesh{
	std::vector<Vertex> vertices;
	std::vector<Index> indices;
};