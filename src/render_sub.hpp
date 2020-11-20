#pragma once

#include <glm/glm.hpp>
#include "data.hpp"
#include "Scene.hpp"

glm::vec3 offset(glm::vec3 pos, glm::vec3 dir){return pos + (dir*1e-3f);}

void tangentspace(glm::vec3 n, glm::vec3 basis[2]){
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

glm::vec3 sampleCosinedHemisphere(float u1, float u2, float* p = nullptr){
	u2 *= 2*kPI;
	float r = sqrt(u1);
	float z = sqrt(1-u1);

	if(p)*p = z/kPI;
	return glm::vec3(r*cos(u2), r*sin(u2), z);
}

glm::vec3 sampleUniformSphere(float u1, float u2, float* p = nullptr){
	u1 = 2*u1 - 1;
	u2 *= 2*kPI;
	float r = sqrt(1-u1*u1);

	if(p)*p = 0.25/kPI;
	return glm::vec3(r*cos(u2), r*sin(u2), u1);
}

glm::vec3 sampleNDF_GGX(float u1, float u2, glm::vec3 wi, float a, float* p = nullptr){
	u2 *= 2*kPI;
	float a2 = a*a;
	float r2 = a2*u1/(1+u1*(a2-1));
	float r = sqrt(r2);
	return glm::vec3(r*cos(u2), r*sin(u2), sqrt(1-r2));
}

float smith_mask(glm::vec3 x, glm::vec3 n, float a){
	float a2 = a*a;
	float xn2 = glm::dot(x,n);
	xn2 *= xn2;
	return 2/(1+sqrt(1+a2*(1-xn2)/xn2));
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
	const Intersection& is, const Material& mtl, const Scene& scene, RNG& rand)
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

		glm::vec3 m_tan = sampleNDF_GGX(rand.uniform(), rand.uniform(), wi, mtl.a);
		glm::vec3 m_w = m_tan.x*tan[0] + m_tan.y*tan[1] + m_tan.z*is.n;

		const glm::vec3 wo = ray.d - 2*glm::dot(ray.d, is.n)*is.n;

		float gi = smith_mask(wi, is.n, mtl.a);
		float go = smith_mask(wo, is.n, mtl.a);
		float w = fabs( gi*go*glm::dot(wi, m_w) / (glm::dot(wi, is.n)*m_tan.z) );

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

Tree createPhotonmap_target(const Scene& scene, int nPhoton, const uint32_t target, RNG& rand){
	std::vector<Photon> photons;
	photons.reserve(10*nPhoton);

	for(int n=0; n<nPhoton; n++){
		const Sphere& source = scene.spheres[scene.lights[0]];
		Ray ray(glm::vec3(0),glm::vec3(0));
		{
			glm::vec3 N = sampleUniformSphere(rand.uniform(), rand.uniform());
			glm::vec3 P = source.p + (source.r + kTINY)*N;
			glm::vec3 tan[2];
			tangentspace(N, tan);

			glm::vec3 hemi = sampleCosinedHemisphere(rand.uniform(), rand.uniform());

			ray.o = offset(P, N);
			ray.d = (N*hemi.z) + (tan[0]*hemi.x) + (tan[1]*hemi.y);
		}
		
		glm::vec3 ph = scene.materials[source.mtlID].color * source.area * kPI / (float)nPhoton;
		// double initialFlux = max(ph);
		float pTerminate = 1;

		while(rand.uniform() < pTerminate){
		// for(int depth=0; depth<5; depth++){
			Intersection is = intersect(ray, scene);
			const Material& mtl = scene.materials[is.mtlID];

			if(mtl.type == Material::Type::EMIT) break;

			if(target == is.mtlID) photons.push_back(Photon(is.p, ph, -ray.d));

			{
				glm::vec3 tan[2];
				tangentspace(is.n, tan);
				glm::vec3 hemi = sampleCosinedHemisphere(rand.uniform(), rand.uniform());

				ray.o = offset(is.p, is.n);
				ray.d = (is.n*hemi.z) + (tan[0]*hemi.x) + (tan[1]*hemi.y);
			}
			
			ph *= mtl.color/pTerminate;

			pTerminate = std::max(mtl.color.x, std::max(mtl.color.y, mtl.color.z));
		}
	}

	Tree tree;
	tree.copyElements(photons.data(), photons.size());
	tree.build();

	return tree;
}

void accumulateRadiance(std::vector<hitpoint>& hitpoints, Tree& photonmap, const Scene& scene, const double alpha){
	#pragma omp parallel for schedule(dynamic)
	// for(hitpoint& hit : hitpoints){
	for(int it=0; it<hitpoints.size(); it++){
		hitpoint& hit = hitpoints[it];
		const Material& mtl = scene.materials[hit.mtlID];

		int M = 0;
		glm::vec3 tauM(0);

		std::vector<Tree::Result> nearPhotons = photonmap.searchNN(hit);
		for(const Tree::Result& p : nearPhotons){
			float photonFilter = 3*(1 - p.distance/hit.R) / (kPI*hit.R*hit.R); // cone
			// double photonFilter = 1/(kPI*hit.R*hit.R); // constant
			tauM += photonFilter * p.photon.ph * mtl.color /kPI; // times BSDF
			M++;
		}

		hit.iteration++;

		if(hit.N==0){
			hit.N += M;
			hit.tau += tauM;
		}
		else{
			int N = hit.N;
			int Nnext = N + alpha*M;

			float ratio = (float)Nnext/(float)(N+M);

			hit.N = Nnext;
			hit.R *= sqrt(ratio);
			hit.tau = (hit.tau + tauM)*ratio;			
		}
	}
}