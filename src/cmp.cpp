#define IMPLEMENT_SPHERE
#define IMPLEMENT_SCENE
#define IMPLEMENT_TREE
#include "cmp.hpp"

#include <stb/stb_image_write.h>

#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <algorithm>

#include "Random.hpp"
#include "kdtree.hpp"
#include "render_sub.hpp"

int createScene(Scene* s){
	s->camera.pos = glm::vec3(0,-10,4);
	s->camera.setDir(glm::vec3(0,1,0), glm::vec3(0,0,1));
	s->camera.flen = 2;

	s->materials[s->environment].type = Material::Type::EMIT;
	s->materials[s->environment].color = glm::vec3(0.05);


	uint32_t light = s->newMaterial(Material::Type::EMIT);
	s->materials[light].color = glm::vec3(30);

	uint32_t white = s->newMaterial(Material::Type::LAMBERT);
	s->materials[white].color = glm::vec3(0.6);

	uint32_t red = s->newMaterial(Material::Type::LAMBERT);
	s->materials[red].color = glm::vec3(0.85, 0.1, 0.1);

	uint32_t green = s->newMaterial(Material::Type::LAMBERT);
	s->materials[green].color = glm::vec3(0.1, 0.85, 0.1);

	uint32_t target = s->newMaterial(Material::Type::LAMBERT);
	s->materials[target].color = glm::vec3(0.6);
	s->cmpTargets.push_back(target);

	// box
	s->add(Sphere(glm::vec3(-1e5, 0, 0), 1e5-4, green)); // left
	s->add(Sphere(glm::vec3( 1e5, 0, 0), 1e5-4, red)); // right
	s->add(Sphere(glm::vec3(0, 0, -1e5), 1e5, white)); // bottom
	s->add(Sphere(glm::vec3(0, 0,  1e5), 1e5-8, white)); // top
	s->add(Sphere(glm::vec3(0,  1e5, 0), 1e5-4, white)); // back
	s->add(Sphere(glm::vec3(1.5, 0.5, 1.5), 1.5, target));
	s->add(Sphere(glm::vec3(0,0,6), 0.5, light)); // light

	return 0;
}

void renderReference(glm::vec3* const result, const int w, const int h, const int spp, const Scene& scene, RNG* const rngs){
	#pragma omp parallel for schedule(dynamic)
	for(int i=0; i<w*h; i++){
		int xi = i%w;
		int yi = i/w;
		RNG& rand = rngs[i];

		for(int n=0; n<spp; n++){
			double x = (double) (2*(xi+rand.uniform())-w )/h;
			double y = (double)-(2*(yi+rand.uniform())-h)/h;

			Ray view = scene.camera.ray(x, y);			
			result[i] += pathTracingKernel_total(view, scene, &rand);
		}
		result[i] /= spp;
	}
}

void renderNonTarget(glm::vec3* const result, const int w, const int h, const int spp, const Scene& scene, RNG* const rngs){
	#pragma omp parallel for schedule(dynamic)
	for(int i=0; i<w*h; i++){
		int xi = i%w;
		int yi = i/w;
		RNG& rand = rngs[i];

		for(int n=0; n<spp; n++){
			double x = (double) (2*(xi+rand.uniform())-w)/h;
			double y = (double)-(2*(yi+rand.uniform())-h)/h;

			Ray view = scene.camera.ray(x, y);			
			result[i] += pathTracingKernel_nonTarget(view, scene, &rand);
		}
		result[i] /= spp;
	}
}

void collectHitpoints(std::vector<hitpoint>& hits,
	const int w, const int h, const int nRay,
	const float R0, const Scene& scene, const uint32_t target, RNG& rng)
{
	for(int i=0; i<w*h*nRay; i++){
		int idx = i/(float)nRay;
		int xi = idx%w;
		int yi = idx/w;

		float x = (float) (2*(xi+rng.uniform())-w)/h;
		float y = (float)-(2*(yi+rng.uniform())-h)/h;
		Ray ray = scene.camera.ray(x, y);
		glm::vec3 throuput(1);
		float pDepth = 1;

		while(rng.uniform()<pDepth){
		// for(int depth=0; depth<5; depth++){
			Intersection is = intersect(ray, scene);
			const Material& mtl = scene.materials[is.mtlID];

			if(mtl.type == Material::Type::EMIT) break;

			if(is.mtlID == target ){
				double p = 0.5;
				if(rng.uniform()<p){
					throuput /= p;
					hits.push_back(hitpoint(is, R0, throuput/(float)nRay, idx, ray));
					break;
				}
				else throuput /= (1-p);
			}


			glm::vec3 tan[2];
			tangentspace(is.n, tan);
			glm::vec3 hemi = sampleCosinedHemisphere(rng.uniform(), rng.uniform());

			ray.o = offset(is.p, is.n);
			ray.d = (is.n*hemi.z) + (tan[0]*hemi.x) + (tan[1]*hemi.y);

			throuput *= mtl.color/pDepth;
			// pDepth = std::min(max(throuput), 1.0);
			pDepth = std::min(1.0f, std::max(throuput.x, std::max(throuput.y, throuput.z)));
		}
	}
}

Tree createPhotonmap(const Scene& scene, int nPhoton, const uint32_t target, RNG* const rand){
	std::vector<Photon> photons;
	photons.reserve(10*nPhoton);

	for(int n=0; n<nPhoton; n++){
		const Sphere& source = scene.spheres[scene.lights[0]];
		Ray ray(glm::vec3(0),glm::vec3(0));
		{
			glm::vec3 N = sampleUniformSphere(rand->uniform(), rand->uniform());
			glm::vec3 P = source.p + (source.r + kTINY)*N;
			glm::vec3 tan[2];
			tangentspace(N, tan);

			glm::vec3 hemi = sampleCosinedHemisphere(rand->uniform(), rand->uniform());

			ray.o = offset(P, N);
			ray.d = (N*hemi.z) + (tan[0]*hemi.x) + (tan[1]*hemi.y);
		}
		
		glm::vec3 ph = scene.materials[source.mtlID].color * source.area * kPI / (float)nPhoton;
		// double initialFlux = max(ph);
		float pTerminate = 1;

		while(rand->uniform() < pTerminate){
		// for(int depth=0; depth<5; depth++){
			Intersection is = intersect(ray, scene);
			const Material& mtl = scene.materials[is.mtlID];

			if(mtl.type == Material::Type::EMIT) break;

			if(target == is.mtlID){
				photons.push_back(Photon(is.p, ph, -ray.d));
				break;
			}

			{
				glm::vec3 tan[2];
				tangentspace(is.n, tan);
				glm::vec3 hemi = sampleCosinedHemisphere(rand->uniform(), rand->uniform());

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