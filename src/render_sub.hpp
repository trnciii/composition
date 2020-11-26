#pragma once

#include <algorithm>
#include <glm/glm.hpp>
#include "data.hpp"
#include "Scene.hpp"

glm::vec3 offset(const glm::vec3& pos, const glm::vec3& dir){return pos + (dir*1e-3f);}

void tangentspace(const glm::vec3 n, glm::vec3 basis[2]){
	int sg =(n.z < 0) ?-1 :1;
	float a = -1.0/(sg+n.z);
	float b = n.x * n.y * a;
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

float GGX_D(const float mn, const float a2){
	float cos2 = mn*mn;
	float t = a2*cos2 + 1-cos2;
	return (t>1e-4)? a2/(kPI*t*t) : 0;
}

float smith_mask(const glm::vec3& x, const glm::vec3& n, const float a2){
	float xn2 = glm::dot(x,n);
	xn2 *= xn2;
	return 2/(1+sqrt(1+a2*(1-xn2)/xn2));
}

// double Fresnel_Schlick(double dot, double ior_in, double ior_out){
// 	double r = pow((ior_in - ior_out)/(ior_in + ior_out),2);
// 	return r + (1-r)*pow(1-dot,5);
// }

float evalBSDF(const glm::vec3& wi, const glm::vec3& wo, const glm::vec3 n, const Material& mtl){
	if(mtl.type == Material::Type::LAMBERT){
		return 1/kPI;
	}

	if(mtl.type == Material::Type::GGX_REFLECTION){
		glm::vec3 m = glm::normalize(wi+wo);
		float a2 = mtl.a*mtl.a;

		float D = GGX_D(glm::dot(n, m), a2);
		float gi = smith_mask(wi, n, a2);
		float go = smith_mask(wo, n, a2);
		float F = 1;
		return F * gi*go * D * 0.25/(fabs(glm::dot(wi,n)*glm::dot(wo,n))+0.001);
	}

	return 0;
}

glm::vec3 sampleCosinedHemisphere(float u1, float u2, float* const p = nullptr){
	u2 *= 2*kPI;
	float r = sqrt(u1);
	float z = sqrt(1-u1);

	if(p)*p = z/kPI;
	return glm::vec3(r*cos(u2), r*sin(u2), z);
}

glm::vec3 sampleUniformSphere(float u1, float u2, float* const p = nullptr){
	u1 = 2*u1 - 1;
	u2 *= 2*kPI;
	float r = sqrt(1-u1*u1);

	if(p) *p = 0.25/kPI;
	return glm::vec3(r*cos(u2), r*sin(u2), u1);
}

glm::vec3 sampleNDF_GGX(float u1, float u2, const float a2, float* const p = nullptr){
	u2 *= 2*kPI;
	float r2 = a2*u1/(1+u1*(a2-1));
	float r = sqrt(r2);
	float z = sqrt(1-r2);

	if(p) *p = GGX_D(z, a2)*z;
	return glm::vec3(r*cos(u2), r*sin(u2), z);
}

Intersection intersect(const Ray& ray, const Scene& scene){
	Intersection is;
		is.dist = kHUGE;
		is.mtlID = scene.environment;

	for(const Sphere& s : scene.spheres)
		s.intersect(&is, ray);

	if(glm::dot(is.n, ray.d)>0){
		is.n *= -1.0f;
		is.backfacing = true;
	}

	return is;
}

void sampleBSDF(Ray& ray, glm::vec3& throuput,
	const Intersection& is, const Material& mtl, const Scene& scene, RNG& rand, float* const p = nullptr)
{
	if(mtl.type == Material::Type::LAMBERT){
		glm::vec3 tan[2];
		tangentspace(is.n, tan);
		glm::vec3 hemi = sampleCosinedHemisphere(rand.uniform(), rand.uniform());

		ray.o = offset(is.p, is.n);
		ray.d = (is.n*hemi.z) + (tan[0]*hemi.x) + (tan[1]*hemi.y);
		throuput *= mtl.color;
	}

	if(mtl.type == Material::Type::GGX_REFLECTION){
		const glm::vec3 wi = -ray.d;
		glm::vec3 tan[2];
		tangentspace(is.n, tan);

		float a2 = mtl.a*mtl.a;

		glm::vec3 m_tan = sampleNDF_GGX(rand.uniform(), rand.uniform(), a2);
		glm::vec3 m_w = m_tan.x*tan[0] + m_tan.y*tan[1] + m_tan.z*is.n;

		const glm::vec3 wo = ray.d - 2*glm::dot(ray.d, m_w)*m_w;

		float gi = smith_mask(wi, is.n, a2);
		float go = smith_mask(wo, is.n, a2);
		float F = 1;
		float w = fabs( F* gi*go * glm::dot(wi, m_w)/(glm::dot(wi, is.n)*m_tan.z) );

		ray.o = offset(is.p, is.n);
		ray.d = wo;
		throuput *= w*mtl.color;
	}
}

glm::vec3 pathTracingKernel_total(Ray ray, const Scene& scene, RNG& rand){
	glm::vec3 throuput(1);
	float pTerminate = 1;

	while(rand.uniform() < pTerminate){
		const Intersection is = intersect(ray, scene);
		const Material& mtl = scene.materials[is.mtlID];

		if(mtl.type == Material::Type::EMIT)
			return throuput*mtl.color;

		sampleBSDF(ray, throuput, is, mtl, scene, rand);
		throuput /= pTerminate;
		pTerminate *= 0.9*std::max(mtl.color.x, std::max(mtl.color.y, mtl.color.z));
	}
	return glm::vec3(0);
}

glm::vec3 pathTracingKernel_nonTarget(Ray ray, const Scene& scene, RNG& rand){
	const std::vector<uint32_t>& targets = scene.cmpTargets;
	glm::vec3 throuput(1);
	float pTerminate = 1;

	while(rand.uniform() < pTerminate){
		const Intersection is = intersect(ray, scene);
		const Material& mtl = scene.materials[is.mtlID];

		if(mtl.type == Material::Type::EMIT)
			return throuput*mtl.color;

		if(std::find(targets.begin(), targets.end(), is.mtlID) != targets.end())
			return glm::vec3(0);

		sampleBSDF(ray, throuput, is, mtl, scene, rand);
		throuput /= pTerminate;
		pTerminate *= 0.9*std::max(mtl.color.x, std::max(mtl.color.y, mtl.color.z));
	}
	return glm::vec3(0);
}