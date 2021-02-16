#pragma once

#include <glm/glm.hpp>
#include "constant.h"
#include "data.hpp"

struct Sphere{
	glm::vec3 p;
	float r;
	float area;
	uint32_t mtlID;

	Sphere(glm::vec3 _p, float _r, uint32_t _m)
	:p(_p), r(_r), mtlID(_m), area(4*kPI*_r*_r){}

	float dist(const Ray& ray)const;
	void intersect(Intersection* is, const Ray& ray)const;
};

#ifdef IMPLEMENT_SPHERE

float Sphere::dist(const Ray& ray)const {
	glm::vec3 OP = p - ray.o;
	float b = glm::dot(ray.d, OP);
	float b2 = b*b;
	float c = glm::dot(OP, OP) - r*r;
	
	if(c < b2){
		float t1 = b - sqrt(b2-c);
		if(0<t1)return t1;

		float t2 = b + sqrt(b2-c);
		if(0<t2)return t2;
	}
	return -1;
}

void Sphere::intersect(Intersection* is, const Ray& ray)const {
	float t = this->dist(ray);
	if((0 <t) && (t< is->dist)){
		is->p = ray.o + t*ray.d;
		is->dist = t;
		is->n = is->ng = (is->p - p)/r;
		is->mtlID = mtlID;
	}
}

#endif