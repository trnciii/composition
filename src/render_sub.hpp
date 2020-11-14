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

glm::vec3 sampleUniformSphere(double u1, double u2, double* p = nullptr){
	u1 = 2*u1 - 1;
	u2 *= 2*kPI;
	double r = sqrt(1-u1*u1);

	if(p)*p = 0.25/kPI;
	return glm::vec3(r*cos(u2), r*sin(u2), u1);
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

glm::vec3 pathTracingKernel_total(Ray ray, const Scene& scene, RNG& rand){
	glm::vec3 throuput(1);
	float pTerminate = 1;

	while(rand.uniform() < pTerminate){
		Intersection is = intersect(ray, scene);
		const Material& mtl = scene.materials[is.mtlID];

		if(mtl.type == Material::Type::EMIT)
			return throuput*mtl.color;

		if(mtl.type == Material::Type::LAMBERT){
			throuput *= mtl.color/pTerminate;

			glm::vec3 tan[2];
			tangentspace(is.n, tan);
			glm::vec3 hemi = sampleCosinedHemisphere(rand.uniform(), rand.uniform());

			ray.o = offset(is.p, is.n);
			ray.d = (is.n*hemi.z) + (tan[0]*hemi.x) + (tan[1]*hemi.y);
		}

		pTerminate = std::max(throuput.x, std::max(throuput.y, throuput.z));
	}
	return glm::vec3(0);
}

glm::vec3 pathTracingKernel_nonTarget(Ray ray, const Scene& scene, RNG& rand){
	const std::vector<uint32_t>& targets = scene.cmpTargets;
	glm::vec3 throuput(1);
	float pTerminate = 1;

	while(rand.uniform() < pTerminate){
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
			glm::vec3 hemi = sampleCosinedHemisphere(rand.uniform(), rand.uniform());

			ray.o = offset(is.p, is.n);
			ray.d = (is.n*hemi.z) + (tan[0]*hemi.x) + (tan[1]*hemi.y);
		}

		pTerminate = std::max(throuput.x, std::max(throuput.y, throuput.z));
	}
	return glm::vec3(0);
}

Tree createPhotonmap(const Scene& scene, int nPhoton, const uint32_t target, RNG& rand){
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
	for(hitpoint& hit : hitpoints){
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