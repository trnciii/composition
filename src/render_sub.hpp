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

glm::vec3 pathTracingKernel_total(Ray ray, const Scene& scene, RNG* const rand){
	glm::vec3 throuput(1);
	float pTerminate = 1;

	while(rand->uniform() < pTerminate){
		Intersection is = intersect(ray, scene);
		const Material& mtl = scene.materials[is.mtlID];

		if(mtl.type == Material::Type::EMIT)
			return throuput*mtl.color;

		if(mtl.type == Material::Type::LAMBERT){
			throuput *= mtl.color/pTerminate;

			glm::vec3 tan[2];
			tangentspace(is.n, tan);
			glm::vec3 hemi = sampleCosinedHemisphere(rand->uniform(), rand->uniform());

			ray.o = offset(is.p, is.n);
			ray.d = (is.n*hemi.z) + (tan[0]*hemi.x) + (tan[1]*hemi.y);
		}

		pTerminate = std::max(throuput.x, std::max(throuput.y, throuput.z));
	}
	return glm::vec3(0);
}

glm::vec3 pathTracingKernel_nonTarget(Ray ray, const Scene& scene, RNG* const rand){
	const std::vector<uint32_t>& targets = scene.cmpTargets;
	glm::vec3 throuput(1);
	float pTerminate = 1;

	while(rand->uniform() < pTerminate){
		Intersection is = intersect(ray, scene);
		const Material& mtl = scene.materials[is.mtlID];

		if(mtl.type == Material::Type::EMIT)
			return throuput*mtl.color;

		if(std::find(targets.begin(), targets.end(), is.mtlID) != targets.end())
			return glm::vec3(0);

		if(mtl.type == Material::Type::LAMBERT){
			throuput *= mtl.color/pTerminate;

			glm::vec3 tan[2];
			tangentspace(is.n, tan);
			glm::vec3 hemi = sampleCosinedHemisphere(rand->uniform(), rand->uniform());

			ray.o = offset(is.p, is.n);
			ray.d = (is.n*hemi.z) + (tan[0]*hemi.x) + (tan[1]*hemi.y);
		}

		pTerminate = std::max(throuput.x, std::max(throuput.y, throuput.z));
	}
	return glm::vec3(0);
}