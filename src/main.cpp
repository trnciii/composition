#include "cmp.hpp"

#include <filesystem>
#include <cassert>
#include <iostream>
#include <bitset>
#include <omp.h>

#include "RenderPass.hpp"
#include "Random.hpp"
#include "print.hpp"
#include "file.hpp"

glm::vec3 colormap_4(double u){
	if(u<0.25) return glm::vec3(0.1 , 0.1 , 0.85);
	if(u<0.5 ) return glm::vec3(0.1 , 0.85, 0.1 );
	if(u<0.75) return glm::vec3(0.85, 0.85, 0.1 );
	return glm::vec3(0.85, 0.1 , 0.1 );
}

int main(void){
	// create output directory.
	// using string for dir-name to later create output filename
	// because the format of path::c_str depends on OS.
	std::string outDir("result");
	if(!( std::filesystem::exists(outDir) && std::filesystem::is_directory(outDir) )){
		std::cout <<"mkdir " <<outDir <<std::endl;
		printBr();
		assert(std::filesystem::create_directory(outDir));
	}
	
	Scene scene;
	createScene(&scene);
	print(scene);

	RNG rand;
	int width = 512;
	int height = 512;
	RenderPass pass(width, height);

	if(scene.targetMaterials.size()==0){
		puts("no target");
		return 0;
	}

	uint32_t pt = pass.addLayer();
	{
		std::cout <<"reference path tracing [" <<pt <<"]" <<std::endl;

		RNG* rngForEveryPixel = new RNG[width*height];
		for(int i=0; i<width*height; i++)
			rngForEveryPixel[i] = RNG(i);

		pathTracing(pass.data(pt), width, height, 200, scene, rngForEveryPixel);
		// writeLayer(pass, reference, outDir + "/reference");
		delete[] rngForEveryPixel;
	}


	uint32_t ppm = pass.addLayer();
	{
		std::cout <<"reference photon mapping [" <<ppm <<"]" <<std::endl;
		int nRay = 4;
		int nPhoton = 10000;
		int iteration = 100;
		float alpha = 0.6;
		float R0 = 1;

		int nThreads = omp_get_max_threads();
		std::vector<RNG> rngs(0);
		for(int i=0; i<nThreads+1; i++)
			rngs.push_back(RNG(i));

		std::vector<hitpoint> hits;
		hits.reserve(width*height*nRay);
		collectHitpoints(hits, pass.width, pass.height, nRay, scene, rngs[nThreads]);
		for(hitpoint& hit : hits)hit.clear(R0);

		for(int i=0; i<iteration; i++){
			Tree photonmap = createPhotonmap(scene, nPhoton, rngs.data(), nThreads);
			accumulateRadiance(hits, photonmap, scene, alpha);
		}

		glm::vec3* image = pass.data(ppm);
		for(hitpoint& hit : hits){
			image[hit.pixel] += hit.tau * hit.weight / (float)iteration;
		}
	}

	// render non target component with pt
	const uint32_t nontarget = pass.addLayer();
	{
		std::cout <<"path tracing for non-target component [" <<nontarget <<"]" <<std::endl;

		RNG* rngForEveryPixel = new RNG[width*height];
		for(int i=0; i<width*height; i++)
			rngForEveryPixel[i] = RNG(i);

		pathTracing_notTarget(pass.data(nontarget), width, height, 1000, scene, rngForEveryPixel);

		delete[] rngForEveryPixel;
		writeLayer(pass, nontarget, outDir + "/nontarget");
	}
	

	// collect hitpoints
	std::vector<std::vector<hitpoint>> hits(scene.targetMaterials.size());
	for(uint32_t i=0; i<scene.targetMaterials.size(); i++){
		int nRay = 512;
		int nDepth = 1;
		RNG rng(0);

		std::cout <<"collecting hitpoints on material " <<scene.targetMaterials[i] <<std::endl;
	
		hits[i].reserve(width*height*nRay);

		// uint32_t targetID = 0;
		collectHitpoints_target_exclusive(hits[i], scene.targetMaterials[i], nDepth,
			pass.width, pass.height, nRay, scene, rng);

		// save hitpoints
		// if(writeVector(hits, outDir + "/hit_2")) std::cout <<"hitpoints saved" <<std::endl;
	}


	// ppm
	// todo: takeover RNG state. currently hits are cleared before this iterations.
	{
		const int nPhoton = 100000;
		const int iteration = 10;
		const float alpha = 0.6;
		const float R0 = 0.5;

		for(std::vector<hitpoint>& vhit : hits) for(hitpoint& hit : vhit) hit.clear(R0);

		const int nThreads = omp_get_max_threads();
		std::vector<RNG> rngs(0);
		for(int i=0; i<nThreads; i++)
			rngs.push_back(RNG(i));

		for(uint32_t n=0; n<iteration; n++){
			const Tree photonmap = createPhotonmap(scene, nPhoton, rngs.data(), nThreads);
			for(std::vector<hitpoint>& h : hits) accumulateRadiance(h, photonmap, scene, alpha);
		}

		for(uint32_t i=0; i<scene.targetMaterials.size(); i++){
			std::string name = "hits" + std::to_string(scene.targetMaterials[i]) + "_glass_64";
			writeVector(hits[i], outDir + "/" + name);
		}
	}

	// composition
	uint32_t composed = pass.addLayer();
	std::cout <<"composed [" <<composed <<"]" <<std::endl;
	for(int i=0; i<pass.length; i++)
		pass.data(composed)[i] = pass.data(nontarget)[i];

	for(uint32_t i=0; i<scene.targetMaterials.size(); i++){
		uint32_t target_raw = pass.addLayer();
		uint32_t target_txr = pass.addLayer();
		std::cout <<"raw target component [" <<target_raw <<"], ";
		std::cout <<"textured target component [" <<target_txr <<"], ";
		

		for(hitpoint hit : hits[i]){
			glm::vec3 tau = hit.tau/(float)(hit.iteration);

			double u = std::max(tau.x, std::max(tau.y, tau.z));
			u = pow(8*u, 1);
			pass.data(target_txr)[hit.pixel] += tau*colormap_4(u) * hit.weight;
			pass.data(target_raw)[hit.pixel] += tau*hit.weight;
		}

		for(int i=0; i<pass.length; i++)
			pass.data(composed)[i] += pass.data(target_txr)[i];
	}


	// save all image
	{
		const int digit = 8;
		std::cout <<"pass output: " <<std::bitset<digit>(writeAllPass(pass, outDir)) <<" / ";
		for(int i=digit; 0<i; --i) std::cout <<(i<=pass.nLayer)? "1" : "0";
		std::cout <<std::endl;
	}

	return 0;
}