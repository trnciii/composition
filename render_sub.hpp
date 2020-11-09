#pragma once

#include <glm/glm.hpp>
#include "data.hpp"
#include "Scene.hpp"

glm::vec3 offset(glm::vec3 pos, glm::vec3 dir){return pos + (dir*1e-6f);}

void tangentspace(glm::vec3 n, glm::vec3 basis[2]){
	int sg =(n.z < 0) ?-1 :1;
	double a = -1.0/(sg+n.z);
	double b = n.x * n.y * a;
	basis[0] = glm::vec3(
		1.0 + sg * n.x*n.x * a,
		sg*b,
		-sg*n.x
	);
	basis[1] = glm::vec3(
		b,
		sg + n.y*n.y * a,
		-n.y
	);
}

glm::vec3 sampleCosinedHemisphere(double u1, double u2, double* p = nullptr){
	u2 *= 2*kPI;
	double r = sqrt(u1);
	double z = sqrt(1-u1);

	if(p)*p = z/kPI;
	return glm::vec3(r*cos(u2), r*sin(u2), z);
}

Intersection intersect(const Ray& ray, const Scene& scene){
	Intersection is;
		is.dist = kHUGE;
		is.mtlID = scene.environment;

	for(const Sphere& s : scene.spheres)
		s.intersect(&is, ray);

	if(glm::dot(is.n, ray.d)>0){
		is.n *= -1;
	}

	return is;
}