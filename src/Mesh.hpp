#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "accel.hpp"

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
	
	Box box;

	void init();
};

#ifdef IMPLEMENT_MESH

void Mesh::init(){
	if(indices.size()<1)return;
	box.init(vertices[indices[0].v0].position);
	for(const Index& index : indices){
		box.update(vertices[index.v0].position);
		box.update(vertices[index.v1].position);
		box.update(vertices[index.v2].position);
	}
}

#endif