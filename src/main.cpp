#include "cmp.hpp"

#include <filesystem>
#include <cassert>
#include <iostream>
#include <bitset>
#include <omp.h>
#include <unordered_map>
#include <chrono>

#include "Image.hpp"
#include "Random.hpp"
#include "file.hpp"
#include "toString.hpp"

glm::vec3 colormap_4(double u){
	if(u<0.25) return glm::vec3(0.1 , 0.1 , 0.85);
	if(u<0.5 ) return glm::vec3(0.1 , 0.85, 0.1 );
	if(u<0.75) return glm::vec3(0.85, 0.85, 0.1 );
	return glm::vec3(0.85, 0.1 , 0.1 );
}

void sampleBSDF(Ray* ray, glm::vec3* throuput,
	const Intersection& is, const Material& mtl,
	RNG& rand, float* const p = nullptr);

glm::vec3 pathTracingKernel(Ray ray, const Scene& scene, RNG& rand);

int main(void){

	std::string outDir("result");
	if(!( std::filesystem::exists(outDir) && std::filesystem::is_directory(outDir) )){
		std::cout <<"mkdir " <<outDir <<std::endl;
		assert(std::filesystem::create_directory(outDir));
	}

	RNG rand;
	glm::dvec2 dim(512, 512);
	std::unordered_map<std::string, Image> images;

	std::cout <<"image size: " <<dim.x <<", " <<dim.y <<std::endl;

	Scene scene;
	createScene(&scene);
	std::cout <<str(scene) <<std::endl;

	if(scene.targetMaterials.size()==0){
		puts("no target");
		return 0;
	}

	{
		Image im(dim.x, dim.y);
		std::cout <<"reference path tracing " <<std::flush;

		RNG* rngForEveryPixel = new RNG[im.len()];
		for(int i=0; i<im.len(); i++)
			rngForEveryPixel[i] = RNG(i);

		auto t0 = std::chrono::high_resolution_clock::now();

		pathTracing(im.data(),im.w, im.h, 200, scene, rngForEveryPixel);

		auto t1 = std::chrono::high_resolution_clock::now();
		std::cout <<std::chrono::duration<double, std::milli>(t1-t0).count() <<" ms" <<std::endl;
		
		im.save(outDir+"/reference");
		delete[] rngForEveryPixel;
	}
	images["pt"] = Image(outDir+"/reference");

	{
		Image im(dim.x, dim.y);
		std::cout <<"reference photon mapping " <<std::flush;

		int nRay = 4;
		int nPhoton = 10000;
		int iteration = 100;
		float alpha = 0.6;
		float R0 = 1;

		int nThreads = omp_get_max_threads();
		std::vector<RNG> rngs(0);
		for(int i=0; i<nThreads+1; i++)
			rngs.push_back(RNG(i));

		auto t0 = std::chrono::high_resolution_clock::now();

		std::vector<hitpoint> hits;
		hits.reserve(im.len()*nRay);
		collectHitpoints(hits, im.w, im.h, nRay, scene, rngs[nThreads]);
		for(hitpoint& hit : hits)hit.clear(R0);

		for(int i=0; i<iteration; i++){
			Tree photonmap = createPhotonmap(scene, nPhoton, rngs.data(), nThreads);
			accumulateRadiance(hits, photonmap, scene, alpha);
		}

		for(hitpoint& hit : hits)
			im.pixels[hit.pixel] += hit.tau * hit.weight / (float)iteration;
		
		auto t1 = std::chrono::high_resolution_clock::now();
		std::cout <<std::chrono::duration<double, std::milli>(t1-t0).count() <<" ms" <<std::endl;

		images["ppm"] = im;
	}

	{
		Image im(dim.x, dim.y);
		std::cout <<"path tracing for non-target component " <<std::flush;

		RNG* rngForEveryPixel = new RNG[im.len()];
		for(int i=0; i<im.len(); i++)
			rngForEveryPixel[i] = RNG(i);

		auto t0 = std::chrono::high_resolution_clock::now();

		pathTracing_notTarget(im.data(), im.w, im.h, 1000, scene, rngForEveryPixel);

		auto t1 = std::chrono::high_resolution_clock::now();
		std::cout <<std::chrono::duration<double, std::milli>(t1-t0).count() <<" ms" <<std::endl;

		delete[] rngForEveryPixel;
		im.save(outDir+"/nontarget");

		im = Image();
		images["nt"] = im.load(outDir+"/nontarget");
	}


	// // collect hitpoints
	std::vector<std::vector<hitpoint>> hits(scene.targetMaterials.size());
	for(uint32_t i=0; i<scene.targetMaterials.size(); i++){
		int nRay = 512;
		int nDepth = 1;
		RNG rng(0);
		uint32_t mtl = scene.targetMaterials[i];

		std::cout <<"collecting hitpoints on " <<scene.materials[mtl].name <<" " <<std::flush;
	
		hits[i].reserve(dim.x*dim.y*nRay/2);

		// uint32_t targetID = 0;

		auto t0 = std::chrono::high_resolution_clock::now();

		collectHitpoints_target_exclusive(hits[i], mtl, nDepth, dim.x, dim.y, nRay, scene, rng);

		auto t1 = std::chrono::high_resolution_clock::now();
		std::cout <<std::chrono::duration<double, std::milli>(t1-t0).count() <<" ms" <<std::endl;

		// save hitpoints
		// if(writeVector(hits, outDir + "/hit_2")) std::cout <<"hitpoints saved" <<std::endl;
	}


	// // ppm
	// // todo: takeover RNG state. currently hits are cleared before this iterations.
	// {
	// 	const int nPhoton = 1000;
	// 	const int iteration = 10;
	// 	const float alpha = 0.6;
	// 	const float R0 = 0.5;

	// 	for(std::vector<hitpoint>& vhit : hits) for(hitpoint& hit : vhit) hit.clear(R0);

	// 	const int nThreads = omp_get_max_threads();
	// 	std::vector<RNG> rngs(0);
	// 	for(int i=0; i<nThreads; i++)
	// 		rngs.push_back(RNG(i));

	// 	for(uint32_t n=0; n<iteration; n++){
	// 		const Tree photonmap = createPhotonmap(scene, nPhoton, rngs.data(), nThreads);
	// 		for(std::vector<hitpoint>& h : hits) accumulateRadiance(h, photonmap, scene, alpha);
	// 	}

	// 	for(uint32_t i=0; i<scene.targetMaterials.size(); i++){
	// 		std::string name = "hits" + std::to_string(scene.targetMaterials[i]) + "_glass_64";
	// 		writeVector(hits[i], outDir + "/" + name);
	// 	}
	// }

	// // pt
	{
		for(std::vector<hitpoint>& target : hits){
			std::cout <<"path tracing from hitpoints on " <<std::endl;
			#pragma omp parallel for schedule(dynamic)
			for(int i=0; i<target.size(); i++){
				hitpoint& hit = target[i];
				RNG rng(i);

				hit.tau = glm::vec3(0);
				hit.iteration = 10 + 500*std::max(hit.weight.x, std::max(hit.weight.y, hit.weight.z));
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
	}

	// // composition
	images["composed"] = images["nt"];
	for(uint32_t i=0; i<scene.targetMaterials.size(); i++){
		std::string raw = "mtl_" + std::to_string(scene.targetMaterials[i]) + "_raw";
		std::string remap = "mtl_" + std::to_string(scene.targetMaterials[i]) + "_remap";
		images[raw] = Image(dim.x, dim.y);
		images[remap] = Image(dim.x, dim.y);

		for(hitpoint hit : hits[i]){
			glm::vec3 tau = hit.tau/(float)(hit.iteration);

			double u = std::max(tau.x, std::max(tau.y, tau.z));
			u = pow(8*u, 1);

			images[remap].pixels[hit.pixel] += tau*colormap_4(u) * hit.weight;
			images[raw].pixels[hit.pixel] += tau*hit.weight;
		}

		for(int i=0; i<dim.x*dim.y; i++)
			images["composed"].pixels[i] += images[remap].pixels[i];
	}

	// save all image
	std::cout <<std::endl;
	for(auto& [key, image] : images){
		std::string name = outDir+"/"+key+".png";
		if(writeImage(image.data(), image.w, image.h, name))
			std::cout <<"saved " <<name <<std::endl;
	}
	std::cout <<std::endl;

	return 0;
}