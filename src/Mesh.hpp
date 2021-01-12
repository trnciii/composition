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
	
	inline std::vector<Vertex> polygon(const Index i)const{
		std::vector<Vertex> res = {
			vertices[i.v0], 
			vertices[i.v1],
			vertices[i.v2]
		};
		return res;
	}

	void intersect_triangle(Intersection* is, const Ray& ray, const Index& index)const;

	inline void intersect(Intersection* is, const Ray& ray)const{
		for(const Index& i : indices){
			intersect_triangle(is, ray, i);
		}
	}

	// tree
	struct Node{
		Box box;

		std::vector<Index>::iterator begin;
		uint32_t size;
		uint32_t next;

		inline Node(const Mesh& mesh,
					const std::vector<Index>::iterator b,
					const std::vector<Index>::iterator e)
		:size(e-b), next(0), begin(b)
		{
			if(size<1)return;

			std::vector<Vertex> ps = mesh.polygon(begin[0]);
			box.init(ps[0].position);

			for(int i=0; i<size; i++){
				for(const Vertex& v : mesh.polygon(begin[i])){
					box.update(v.position);
				}
			}
		}
	};


	Box box;
	std::vector<Node> nodes;
	const uint32_t nElements = 20;

	void update();
};


#ifdef IMPLEMENT_MESH

void Mesh::intersect_triangle(Intersection* is, const Ray& ray, const Index& index)const{
	const std::vector<Vertex> v = polygon(index);

	glm::vec3 e0 = v[1].position - v[0].position;
	glm::vec3 e1 = v[2].position - v[0].position;
	glm::vec3 o = ray.o - v[0].position;
	glm::vec3 q = glm::cross(e0, o);
	glm::vec3 p = glm::cross(e1, ray.d);
	float _det = 1/glm::dot(e0, p);

	float s = glm::dot(o, p)*_det;
	float t = glm::dot(ray.d, q)*_det;
	float d = (0<s && 0<t && s+t<1)? glm::dot(q, e1)*_det : -1;

	if(0<d && d<is->dist){
		is->dist = d;
		is->p = ray.o + d*ray.d;
		is->n = glm::normalize((1-s-t)*v[0].normal + s*v[1].normal + t*v[2].normal);
		is->mtlID = index.mtlID;
	}
}

void Mesh::update(){
	if(indices.size()<1)return;
	box.update(vertices[indices[0].v0].position);
	for(const Index& index : indices){
		box.update(vertices[index.v0].position);
		box.update(vertices[index.v1].position);
		box.update(vertices[index.v2].position);
	}
}

#endif