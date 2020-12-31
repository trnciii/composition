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

#pragma omp declare reduction (merge:\
std::vector<Photon>, std::vector<hitpoint>:\
omp_out.insert(omp_out.end(), omp_in.begin(), omp_in.end()) )

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

	uint32_t target1 = s->newMaterial(Material::Type::GLASS);
	// uint32_t target1 = s->newMaterial(Material::Type::LAMBERT);
	s->materials[target1].color = glm::vec3(1);
	s->materials[target1].a = 0.02;
	s->materials[target1].ior = 2;
	s->cmpTargets.push_back(target1);

	uint32_t target2 = s->newMaterial(Material::Type::GGX_REFLECTION);
	s->materials[target2].color = glm::vec3(1);
	s->materials[target2].a = 0.04;
	s->cmpTargets.push_back(target2);

	// box
	s->add(Sphere(glm::vec3(-1e4, 0, 0), 1e4-4, green)); // left
	s->add(Sphere(glm::vec3( 1e4, 0, 0), 1e4-4, red)); // right
	s->add(Sphere(glm::vec3(0, 0, -1e4), 1e4, white)); // bottom
	s->add(Sphere(glm::vec3(0, 0,  1e4), 1e4-8, white)); // top
	s->add(Sphere(glm::vec3(0,  1e4, 0), 1e4-4, white)); // back
	s->add(Sphere(glm::vec3( 1.5, 0.0, 1.2), 1.2, target1));
	s->add(Sphere(glm::vec3(-1.5, 1.5, 1.5), 1.5, target2));
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
			double x = (double) (2*(xi+rand.uniform())-w)/h;
			double y = (double)-(2*(yi+rand.uniform())-h)/h;

			Ray view = scene.camera.ray(x, y);			
			result[i] += pathTracingKernel_total(view, scene, rand);
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
			result[i] += pathTracingKernel_nonTarget(view, scene, rand);
		}
		result[i] /= spp;
	}
}

void collectHitpoints_all(std::vector<hitpoint>& hits,
	const int w, const int h, const int nRay,
	const Scene& scene, RNG& rng)
{
	// #pragma omp parallel for reduction(merge: hits) schedule(dynamic)
	for(int i=0; i<w*h*nRay; i++){
		int p = i/nRay;
		float x = (float) (2*((p%w)+rng.uniform())-w)/h;
		float y = (float)-(2*((p/w)+rng.uniform())-h)/h;
		Ray ray = scene.camera.ray(x, y);

		const Intersection is = intersect(ray, scene);

		if( scene.materials[is.mtlID].type != Material::Type::EMIT )
			hits.push_back(hitpoint(is, glm::vec3(1.0/(float)nRay), p, ray, 1));
	}
}

void collectHitpoints_target(std::vector<hitpoint>& hits, 
	const uint32_t targetID, const int targetDepth,
	const int w, const int h, const int nRay,
	const Scene& scene, RNG& rng)
{
	std::vector<uint32_t> others = scene.cmpTargets;
	others.erase(others.begin()+targetID);

	// #pragma omp parallel for reduction(merge: hits) schedule(dynamic)
	for(int i=0; i<w*h*nRay; i++){
		int p = i/nRay;
		float x = (float) (2*((p%w)+rng.uniform())-w)/h;
		float y = (float)-(2*((p/w)+rng.uniform())-h)/h;
		
		Ray ray = scene.camera.ray(x, y);
		glm::vec3 throuput(1);
		
		float pTerminate = 1;
		int d_all = 0;
		int countTargetDepth = 0;

		while(rng.uniform()<pTerminate){
			d_all++;
			const Intersection is = intersect(ray, scene);
			const Material& mtl = scene.materials[is.mtlID];

			if( mtl.type == Material::Type::EMIT ) break;

			if( is.mtlID == scene.cmpTargets[targetID] ){
				countTargetDepth++;

				if( countTargetDepth == targetDepth ){
					hits.push_back(hitpoint(is, throuput/(float)nRay, p, ray, d_all));
					break;
				}
			}

			sampleBSDF(ray, throuput, is, mtl, scene, rng);
			throuput /= pTerminate;
			pTerminate *= std::max(mtl.color.x, std::max(mtl.color.y, mtl.color.z));
			if(10<d_all) pTerminate *= 0.5;
		}
	}
}

void collectHitpoints_target_one(std::vector<hitpoint>& hits,
	const uint32_t targetID, const int targetDepth,
	const int w, const int h, const int nRay,
	const Scene& scene, RNG& rng)
{
	std::vector<uint32_t> others = scene.cmpTargets;
	others.erase(others.begin()+targetID);

	// #pragma omp parallel for reduction(merge: hits) schedule(dynamic)
	for(int i=0; i<w*h*nRay; i++){
		int p = i/nRay;
		float x = (float) (2*((p%w)+rng.uniform())-w)/h;
		float y = (float)-(2*((p/w)+rng.uniform())-h)/h;
		
		Ray ray = scene.camera.ray(x, y);
		glm::vec3 throuput(1);
		
		float pTerminate = 1;
		int d_all = 0;
		int countTargetDepth = 0;
		int countTargetDepth_others = 0;

		hitpoint firstMe, firstOthers;
		bool hasHitMe = false;
		bool hasHitOthers = false;
		int targetDepth_others = 1;

		while(rng.uniform()<pTerminate){
		// for(int depth=0; depth<5; depth++){
			d_all++;
			const Intersection is = intersect(ray, scene);
			const Material& mtl = scene.materials[is.mtlID];

			if( mtl.type == Material::Type::EMIT ) break;

			if( is.mtlID == scene.cmpTargets[targetID] ){
				countTargetDepth++;

				if( countTargetDepth == targetDepth ){
					hasHitMe = true;
					firstMe = hitpoint(is, throuput/(float)nRay, p, ray, d_all);
					// break;
				}
			}
			else if(std::find(others.begin(), others.end(), is.mtlID) != others.end()){
				countTargetDepth_others++;
				if(countTargetDepth_others == targetDepth_others){
					hasHitOthers = true;
					firstOthers = hitpoint(is, throuput/(float)nRay, p, ray, d_all);
				}
			}

			if(hasHitOthers && hasHitMe) break;

			sampleBSDF(ray, throuput, is, mtl, scene, rng);
			throuput /= pTerminate;
			pTerminate *= std::max(mtl.color.x, std::max(mtl.color.y, mtl.color.z));
			if(10<d_all) pTerminate *= 0.5;
		}

		if(hasHitMe && !hasHitOthers) hits.push_back(firstMe);
		else if(hasHitMe && hasHitOthers){
			if(firstMe.depth>1)hits.push_back(firstMe);
			else if(firstOthers.depth == 1 && firstMe.depth == 2)hits.push_back(firstMe);
		}

	}
}

Tree createPhotonmap_all(const Scene& scene, int nPhoton, RNG& rand){
	std::vector<Photon> photons;
	photons.reserve(10*nPhoton);

	const Sphere& source = scene.spheres[scene.lights[0]];

	// #pragma omp parallel for reduction(merge: photons) schedule(dynamic)
	for(int n=0; n<nPhoton; n++){
		glm::vec3 ro, rd;
		{
			glm::vec3 N = sampleUniformSphere(rand.uniform(), rand.uniform());
			glm::vec3 P = source.p + (source.r + kTINY)*N;
			glm::vec3 tan[2];
			tangentspace(N, tan);

			glm::vec3 hemi = sampleCosinedHemisphere(rand.uniform(), rand.uniform());

			ro = offset(P, N);
			rd = (N*hemi.z) + (tan[0]*hemi.x) + (tan[1]*hemi.y);
		}
		Ray ray(ro,rd);
		
		glm::vec3 ph = scene.materials[source.mtlID].color * source.area * kPI / (float)nPhoton;
		float pTerminate = 1;
		int depth = 0;

		while(rand.uniform() < pTerminate){
		// for(int depth=0; depth<5; depth++){
			depth++;
			Intersection is = intersect(ray, scene);
			const Material& mtl = scene.materials[is.mtlID];

			if(mtl.type == Material::Type::EMIT) break;

			photons.push_back(Photon(is.p, ph, -ray.d));

			sampleBSDF(ray, ph, is, mtl, scene, rand);
			ph /= pTerminate;
			pTerminate *= std::max(mtl.color.x, std::max(mtl.color.y, mtl.color.z));
			if(10<depth) pTerminate *= 0.5; 
		}
	}

	Tree tree;
	tree.copyElements(photons.data(), photons.size());
	tree.build();

	return tree;
}

Tree createPhotonmap_target(const Scene& scene, int nPhoton, const uint32_t targetID, RNG& rand){
	std::vector<Photon> photons;
	photons.reserve(10*nPhoton);

	const Sphere& source = scene.spheres[scene.lights[0]];

	// #pragma omp parallel for reduction(merge: photons) schedule(dynamic)
	for(int n=0; n<nPhoton; n++){
		glm::vec3 ro, rd;
		{
			glm::vec3 N = sampleUniformSphere(rand.uniform(), rand.uniform());
			glm::vec3 P = source.p + (source.r + kTINY)*N;
			glm::vec3 tan[2];
			tangentspace(N, tan);

			glm::vec3 hemi = sampleCosinedHemisphere(rand.uniform(), rand.uniform());

			ro = offset(P, N);
			rd = (N*hemi.z) + (tan[0]*hemi.x) + (tan[1]*hemi.y);
		}
		Ray ray(ro,rd);
		
		glm::vec3 ph = scene.materials[source.mtlID].color * source.area * kPI / (float)nPhoton;
		float pTerminate = 1;
		int depth = 0;

		while(rand.uniform() < pTerminate){
		// for(int depth=0; depth<5; depth++){
			depth++;
			Intersection is = intersect(ray, scene);
			const Material& mtl = scene.materials[is.mtlID];

			if(mtl.type == Material::Type::EMIT) break;

			if(scene.cmpTargets[targetID] == is.mtlID) photons.push_back(Photon(is.p, ph, -ray.d));

			sampleBSDF(ray, ph, is, mtl, scene, rand);
			ph /= pTerminate;
			pTerminate *= std::max(mtl.color.x, std::max(mtl.color.y, mtl.color.z));
			if(10<depth) pTerminate *= 0.5;
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
			tauM += photonFilter * p.photon.ph * mtl.color * evalBSDF(p.photon.wi, hit.wo, hit.n, mtl);
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