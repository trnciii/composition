#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "accel.hpp"

#include <iostream>

struct Vertex{
	glm::vec3 position;
	glm::vec3 normal;
};

struct Index{
	uint32_t v0;
	uint32_t v1;
	uint32_t v2;

	glm::vec3 normal;
	bool use_smooth;

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
		uint32_t i = 0;
		while(i < nodes.size()){
			float dist_box = nodes[i].box.distance(ray);
			if( 0 < dist_box && dist_box < is->dist){
				if(nodes[i].size <= nElements){
					for(int j=nodes[i].begin; j < nodes[i].begin + nodes[i].size; j++){
						const Index& I = indices[j];
						// std::cout <<I.v0 <<", " <<I.v1 <<", " <<I.v2 <<std::endl;
 
						intersect_triangle(is, ray, indices[j]);
					}
				}
				i++;
			}
			else i += nodes[i].next;
		}
	}

	// tree
	struct Node{
		Box box;
		
		const uint32_t begin;
		const uint32_t size;
		uint32_t next;

		inline Node(const Mesh* const mesh,
					const uint32_t b,
					const uint32_t e)
		:next(0), begin(b), size(e-b)
		{
			if(e-b<1)return;

			std::vector<Vertex> ps = mesh->polygon(mesh->indices[begin]);
			box.init(ps[0].position);

			for(int i=begin; i<begin+size; i++){
				for(const Vertex& v : mesh->polygon(mesh->indices[i])){
					box.update(v.position);
				}
			}
		}
	};

	std::vector<Node> nodes;
	const uint32_t nElements = 10;

	void update();
	void split(const uint32_t polygon_begin, const uint32_t polygon_end, const int axis);
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

	if(0 < d && d < is->dist){
		is->dist = d;
		is->p = ray.o + d*ray.d;
		is->ng = index.normal;
		is->n = index.use_smooth? glm::normalize((1-s-t)*v[0].normal + s*v[1].normal + t*v[2].normal) : index.normal;
		is->mtlID = index.mtlID;
	}
}

void Mesh::split(const uint32_t begin, const uint32_t end, const int axis){
	// split by counting the center of mass for each polygon
	std::sort(indices.begin()+begin, indices.begin()+end,
		[axis, this](const Index& a, const Index& b){
			std::vector<Vertex> plya = polygon(a);
			std::vector<Vertex> plyb = polygon(b);
			return (plya[0].position[axis] + plya[1].position[axis] + plya[2].position[axis])
				< (plyb[0].position[axis] + plyb[1].position[axis] + plyb[2].position[axis]);
		});
	uint32_t mid = begin + (end - begin)/2;

	uint32_t p0 = nodes.size();
	{
		Node node(this, begin, mid);
		nodes.push_back(node);
		if(nElements < node.size)split(begin, mid, node.box.axis());
	}
	
	uint32_t p1 = nodes.size();
	{
		Node node(this, mid, end);
		nodes.push_back(node);
		if(nElements < node.size)split(mid, end, node.box.axis());
	}

	uint32_t p2 = nodes.size();

	nodes[p0].next = p1 - p0;
	nodes[p1].next = p2 - p1;
}

void Mesh::update(){
	nodes.clear();
	if(indices.size()<1) return;

	Node root(this, 0, indices.size());
	nodes.push_back(root);
	
	if(nElements < root.size)
		split(0, indices.size(), root.box.axis());

	nodes[0].next = nodes.size();
}

#endif