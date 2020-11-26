#pragma once

#include <glm/glm.hpp>

struct Ray;
struct Material;
struct Intersection;
struct hitpoint;
struct Photon;

struct Ray{
	glm::vec3 o;
	glm::vec3 d;

	inline Ray(glm::vec3 _o, glm::vec3 _d):o(_o), d(_d){}
};

struct Material{

	enum Type{
		EMIT,
		LAMBERT,
		// DIELECTRIC,
		GGX_REFLECTION,
		// GGX_REFRACTOIN,
	};

	Type type;
	glm::vec3 color = glm::vec3(0.6, 0.6, 0.6);
	float a = 0;
	// double ior;
	
	inline Material(Material::Type t):type(t){}
};

// ray - object intersection info
struct Intersection{
	float dist;
	glm::vec3 p;
	glm::vec3 n;
	uint32_t mtlID;
	bool backfacing = false;
};

// first hit point from eye (following the ppm paper)
struct hitpoint{
	glm::vec3 p;
	glm::vec3 n;
	glm::vec3 wo; // ray direction << done
	uint32_t mtlID;
	uint32_t pixel;
	float R; // current photon radius <<todo
	int N = 0; // accumulated photon count <<todo
	glm::vec3 tau = glm::vec3(0,0,0); // accumulated reflected flux <<todo
	glm::vec3 weight;
	int iteration = 0;
	// int targetID;
	int depth;

	inline hitpoint(){}
	
	inline hitpoint(const Intersection& is,
		float r, glm::vec3 w, uint32_t px, Ray& ray, int d/*, int tID*/)
	:p(is.p), n(is.n), wo(-ray.d), mtlID(is.mtlID), pixel(px),
	R(r), weight(w), depth(d)/*, targetID(tID)*/{}

	inline void clear(float R0){
		N = 0;
		iteration = 0;
		R = R0;
		tau = glm::vec3(0);
	}
};

struct Photon{
	glm::vec3 p;
	glm::vec3 ph;
	glm::vec3 wi;

	inline Photon(glm::vec3 _p, glm::vec3 _ph, glm::vec3 _wi)
	:p(_p), ph(_ph), wi(_wi){}
};