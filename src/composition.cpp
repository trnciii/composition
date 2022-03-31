#define IMPLEMENT_SPHERE
#define IMPLEMENT_TREE
#define IMPLEMENT_MESH

#include "composition.hpp"

#include <omp.h>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <algorithm>

#include "render_sub.hpp"

#pragma omp declare reduction (merge:\
std::vector<Photon>, std::vector<hitpoint>:\
omp_out.insert(omp_out.end(), omp_in.begin(), omp_in.end()) )

std::vector<RNG> createRNGVector(int len, int offset){
	std::vector<RNG> rngs;
	for(int i=0; i<len; i++) rngs.push_back(RNG(i+offset));
	return rngs;
}

int createScene(Scene*const s){
	s->camera.position = glm::vec3(0, -10, 4);
	s->camera.setDirection(glm::vec3(0,1,0), glm::vec3(0,0,1));
	s->camera.flen = 2;

	s->materials[s->environment].type = Material::Type::EMIT;
	s->materials[s->environment].color = glm::vec3(0.05);

	Material mLight;
		mLight.name = "light";
		mLight.type = Material::Type::EMIT;
		mLight.color = glm::vec3(30);

	uint32_t light = s->addMaterial(mLight);

	Material mWhite;
		mWhite.name = "white";
		mWhite.type = Material::Type::LAMBERT;
		mWhite.color = glm::vec3(0.6);

	uint32_t white = s->addMaterial(mWhite);

	Material mRed;
		mRed.name = "red";
		mRed.type = Material::Type::LAMBERT;
		mRed.color = glm::vec3(0.85, 0.1, 0.1);

	uint32_t red = s->addMaterial(mRed);

	Material mGreen;
		mGreen.type = Material::Type::LAMBERT;
		mGreen.color = 	glm::vec3(0.1, 0.85, 0.1);

	uint32_t green = s->addMaterial(mGreen);

	Material mTarget1;
		mTarget1.name = "right";
		mTarget1.type = Material::Type::GLASS;
		mTarget1.color = glm::vec3(1);
		mTarget1.a = 0.1;

	uint32_t target1 = s->addMaterial(mTarget1);
	s->targetMaterials.push_back(target1);

	Material mTarget2;
		mTarget2.name = "left";
		mTarget2.type = Material::Type::GGX_REFLECTION;
		mTarget2.color = glm::vec3(1);
		mTarget2.a = 0.04;

	uint32_t target2 = s->addMaterial(mTarget2);
	s->targetMaterials.push_back(target2);

	Material mFloor;
		mFloor.type = Material::Type::GGX_REFLECTION;
		mFloor.color = glm::vec3(0.5);
		mFloor.a = 0.01;

	uint32_t floor = s->addMaterial(mFloor);

	// box
	s->addSphere(glm::vec3(-1e4, 0, 0), 1e4-4, green, "left");
	s->addSphere(glm::vec3( 1e4, 0, 0), 1e4-4, red, "right");
	s->addSphere(glm::vec3(0, 0, -1e4), 1e4, white, "floor");
	s->addSphere(glm::vec3(0, 0,  1e4), 1e4-8, white, "ceil");
	s->addSphere(glm::vec3(0,  1e4, 0), 1e4-4, white, "bask");
	s->addSphere(glm::vec3( 1.5, 0.0, 1.2), 1.2, target1); // right
	s->addSphere(glm::vec3(-1.5, 1.5, 1.5), 1.5, target2); // left
	s->addSphere(glm::vec3(0,0,6), 0.5, light, "light");

	Mesh m;
	{
		m.name = "plane";
		
		std::vector<Vertex> v = {
			{{-4, 0, 6}, {0, -1, 0}},
			{{-4, 0, 2}, {0, -1, 0}},
			{{ 0, 0, 2}, {0, -1, 0}},
			{{ 0, 0, 6}, {0, -1, 0}},
			{{ 4, 0, 6}, {0, -1, 0}},
			{{ 4, 0, 2}, {0, -1, 0}}
		};

		std::vector<Index> i = {
			{0, 1, 2, glm::normalize(glm::cross(v[1].position-v[0].position, v[2].position-v[0].position)), false, green},
			{2, 3, 0, glm::normalize(glm::cross(v[3].position-v[2].position, v[0].position-v[2].position)), false, red},
			{3, 2, 5, glm::normalize(glm::cross(v[2].position-v[3].position, v[5].position-v[3].position)), false, green},
			{5, 4, 3, glm::normalize(glm::cross(v[4].position-v[5].position, v[3].position-v[5].position)), false, red}
		};
		
		m.vertices.resize(v.size());
		std::copy(v.begin(), v.end(), m.vertices.begin());

		m.indices.resize(i.size());
		std::copy(i.begin(), i.end(), m.indices.begin());

		m.buildTree();	
	}

	// s->addMesh(m);
	return 0;
}


Image PT(int w, int h, const Scene& scene, const int spp,
	std::vector<RNG>& rng_per_pixel)
{
	Image image(w, h);

	#pragma omp parallel for schedule(dynamic)
	for(int i=0; i<w*h; i++){
		int xi = i%w;
		int yi = i/w;
		RNG& rng = rng_per_pixel[i];

		for(int n=0; n<spp; n++){
			double x = (double) (2*(xi+rng.uniform())-w)/h;
			double y = (double)-(2*(yi+rng.uniform())-h)/h;

			Ray view = scene.camera.ray(x, y);			
			image.pixels[i] += pathTracingKernel(view, scene, rng);
		}
		image.pixels[i] /= spp;
	}

	return image;
}

Image PT_notTarget(const int w, const int h, const Scene& scene,
	const int spp, std::vector<RNG>& rng_per_pixel)
{
	Image image(w, h);
	
	#pragma omp parallel for schedule(dynamic)
	for(int i=0; i<w*h; i++){
		int xi = i%w;
		int yi = i/w;
		RNG& rng = rng_per_pixel[i];

		for(int n=0; n<spp; n++){
			double x = (double) (2*(xi+rng.uniform())-w)/h;
			double y = (double)-(2*(yi+rng.uniform())-h)/h;

			Ray view = scene.camera.ray(x, y);			
			image.pixels[i] += pathTracingKernel_notTarget(view, scene, rng);
		}
		image.pixels[i] /= spp;
	}

	return image;
}

std::vector<hitpoint> collectHitpoints(const int w, const int h,
	const int nRay, const Scene& scene, std::vector<RNG>& rng_per_thread)
{
	std::vector<hitpoint> hits;

	#pragma omp parallel for reduction(merge: hits) schedule(dynamic)\
	num_threads(rng_per_thread.size())
	for(int i=0; i<w*h*nRay; i++){
		int p = i/nRay;
		RNG& rng = rng_per_thread[omp_get_thread_num()];

		float x = (float) (2*((p%w)+rng.uniform())-w)/h;
		float y = (float)-(2*((p/w)+rng.uniform())-h)/h;
		Ray ray = scene.camera.ray(x, y);

		const Intersection is = intersect(ray, scene);

		if( scene.materials[is.mtlID].type != Material::Type::EMIT )
			hits.push_back(hitpoint(is, glm::vec3(1.0/(float)nRay), p, ray, 1));
	}

	return hits;
}

std::vector<hitpoint> collectHitpoints_target
(	const uint32_t tMtl, const int targetDepth,
	const int w, const int h, const int nRay,
	const Scene& scene, std::vector<RNG>& rng_per_thread)
{
	std::vector<hitpoint> hits;
	std::vector<uint32_t> others;
	for(uint32_t i : scene.targetMaterials)if(i!=tMtl)others.push_back(i);

	#pragma omp parallel for reduction(merge: hits) schedule(dynamic)\
	num_threads(rng_per_thread.size())
	for(int i=0; i<w*h*nRay; i++){
		int p = i/nRay;
		RNG& rng = rng_per_thread[omp_get_thread_num()];

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

			if( is.mtlID == tMtl ){
				countTargetDepth++;

				if( countTargetDepth == targetDepth ){
					hits.push_back(hitpoint(is, throuput/(float)nRay, p, ray, d_all));
					break;
				}
			}

			sampleBSDF(&ray, &throuput, is, mtl, rng);
			throuput /= pTerminate;
			pTerminate *= std::max(mtl.color.x, std::max(mtl.color.y, mtl.color.z));
			if(10<d_all) pTerminate *= 0.5;
		}
	}

	return hits;
}

std::vector<hitpoint> collectHitpoints_target_exclusive
(	const uint32_t tMtl, const int targetDepth,
	const int w, const int h, const int nRay,
	const Scene& scene, std::vector<RNG>& rng_per_thread)
{
	std::vector<hitpoint> hits;
	std::vector<uint32_t> others;
	for(uint32_t i : scene.targetMaterials)if(i != tMtl)others.push_back(i);

	#pragma omp parallel for reduction(merge: hits) schedule(dynamic)\
	num_threads(rng_per_thread.size())
	for(int i=0; i<w*h*nRay; i++){
		int p = i/nRay;
		RNG& rng = rng_per_thread[omp_get_thread_num()];

		float x = (float) (2*((p%w)+rng.uniform())-w)/h;
		float y = (float)-(2*((p/w)+rng.uniform())-h)/h;
		
		Ray ray = scene.camera.ray(x, y);
		glm::vec3 throuput(1);
		
		float pTerminate = 1;
		int depth_all = 0;
		int countTargetDepth = 0;
		int countTargetDepth_others = 0;

		hitpoint firstMe, firstOthers;
		bool hasHitMe = false;
		bool hasHitOthers = false;
		int targetDepth_others = 1;

		while(rng.uniform()<pTerminate){
		// for(int depth=0; depth<5; depth++){
			depth_all++;
			const Intersection is = intersect(ray, scene);
			const Material& mtl = scene.materials[is.mtlID];

			if( mtl.type == Material::Type::EMIT ) break;

			if( is.mtlID == tMtl ){
				countTargetDepth++;

				if( countTargetDepth == targetDepth ){
					hasHitMe = true;
					firstMe = hitpoint(is, throuput/(float)nRay, p, ray, depth_all);
					// break;
				}
			}
			else if(std::find(others.begin(), others.end(), is.mtlID) != others.end()){
				countTargetDepth_others++;
				if(countTargetDepth_others == targetDepth_others){
					hasHitOthers = true;
					firstOthers = hitpoint(is, throuput/(float)nRay, p, ray, depth_all);
				}
			}

			if(hasHitOthers && hasHitMe) break;

			sampleBSDF(&ray, &throuput, is, mtl, rng);
			throuput /= pTerminate;
			pTerminate *= std::max(mtl.color.x, std::max(mtl.color.y, mtl.color.z));
			if(10<depth_all) pTerminate *= 0.5;
		}

		if(hasHitMe && !hasHitOthers) hits.push_back(firstMe);
		else if(hasHitMe && hasHitOthers){
			if(firstMe.depth<firstOthers.depth
			&& scene.materials[firstMe.mtlID].type == Material::Type::LAMBERT){
				hits.push_back(firstMe);
			}
			else if(firstMe.depth>1)hits.push_back(firstMe);
			else if(firstOthers.depth == 1 && firstMe.depth == 2)hits.push_back(firstMe);
		}
	}

	return hits;
}

Tree createPhotonmap(const Scene& scene, const int nPhoton, std::vector<RNG>& rng_per_thread){
	if(scene.lights.size()==0) return Tree();

	std::vector<Photon> photons;
	photons.reserve(10*nPhoton);

	const Sphere& source = scene.spheres[scene.lights[0]];

	#pragma omp parallel for reduction(merge: photons) schedule(dynamic)\
	num_threads(rng_per_thread.size())
	for(int n=0; n<nPhoton; n++){
		RNG& rand = rng_per_thread[omp_get_thread_num()];

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

			sampleBSDF(&ray, &ph, is, mtl, rand);
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

void progressiveRadianceEstimate(std::vector<hitpoint>& hitpoints, const Tree& photonmap,
	const Scene& scene, const double alpha)
{
	#pragma omp parallel for schedule(dynamic)
	for(hitpoint& hit : hitpoints){
		const Material& mtl = scene.materials[hit.mtlID];

		int M = 0;
		glm::vec3 tauM(0);

		std::vector<Tree::Result> nearPhotons = photonmap.searchNN(hit);
		for(const Tree::Result& p : nearPhotons){
			float filter = 3*(1 - p.distance/hit.R) / (kPI*hit.R*hit.R); // cone
			// float filter = 1/(kPI*hit.R*hit.R); // constant
			tauM += filter * p.photon.ph * mtl.color * evalBSDF(p.photon.wi, hit.wo, hit.n, mtl);
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

Image PPM(const int w, const int h, const Scene& scene,
	const int nRay, const int nPhoton, const int iteration, const float alpha, const float R0,
	std::vector<RNG>& rng_per_thread)
{

	std::cout <<"collecting hitpoints" <<std::endl;
	std::vector<hitpoint> hits = collectHitpoints(w, h, nRay, scene, rng_per_thread);
	for(hitpoint& hit : hits)hit.clear(R0);
	
	std::cout <<"radiance estimation" <<std::endl;
	radiance_PPM(hits, scene, nPhoton, iteration, alpha, rng_per_thread);
	
	Image im(w, h);
	for(hitpoint& hit : hits)
		im.pixels[hit.pixel] += hit.tau * hit.weight / (float)iteration;
	return im;
}

void radiance_PPM(std::vector<hitpoint>& hits, const Scene& scene,
	const int nPhoton, const int iteration, const float alpha,
	std::vector<RNG>& rng_per_thread)
{
	for(int i=0; i<iteration; i++){
		Tree photonmap = createPhotonmap(scene, nPhoton, rng_per_thread);
		progressiveRadianceEstimate(hits, photonmap, scene, alpha);
	}
}

void radiance_PT(std::vector<hitpoint>& hits, const Scene& scene,
	const int spp, std::vector<RNG>& rng_per_thread)
{
	#pragma omp parallel for schedule(dynamic) num_threads(rng_per_thread.size())
	for(int i=0; i<hits.size(); i++){
		hitpoint& hit = hits[i];
		RNG& rng = rng_per_thread[omp_get_thread_num()];

		hit.tau = glm::vec3(0);
		hit.iteration = 10 + spp*std::max(hit.weight.x, std::max(hit.weight.y, hit.weight.z));
		for(int j=0; j<hit.iteration; j++){
			glm::vec3 th(1);
			Ray ray(glm::vec3(0), -hit.wo);
			Intersection is;
				is.dist = kHUGE;
				is.p = hit.p;
				is.n = hit.n;
				is.ng = hit.ng;
				is.mtlID = hit.mtlID;
				is.backfacing = false; // ?

			sampleBSDF(&ray, &th, is, scene.materials[is.mtlID], rng);
			hit.tau += th * pathTracingKernel(ray, scene, rng);
		}
	}
}

Image nprr(const int w, const int h, const Scene& scene,
	const int spp, std::vector<RNG>& rng_per_pixel,
	const std::vector<std::function<glm::vec3(float)>>& remaps)
{
	Image image(w, h);

	#pragma omp parallel for schedule(dynamic)
	for(int i=0; i<w*h; i++){
		int xi = i%w;
		int yi = i/w;
		RNG& rng = rng_per_pixel[i];

		for(int n=0; n<spp; n++){
			double x = (double) (2*(xi+rng.uniform())-w)/h;
			double y = (double)-(2*(yi+rng.uniform())-h)/h;

			Ray view = scene.camera.ray(x, y);
			image.pixels[i] += nprrKernel(view, scene, rng, remaps);
		}
		image.pixels[i] /= spp;
	}

	return image;
}