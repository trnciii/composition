#include "cmp.hpp"

#include <filesystem>
#include <cassert>
#include <iostream>
#include <bitset>

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

	if(scene.cmpTargets.size()==0){
		puts("no target");
		return 0;
	}

	// // render path traced reference
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

	{
		const int digit = 8;
		std::cout <<"pass output: " <<std::bitset<digit>(writeAllPass(pass, outDir)) <<" / ";
		for(int i=digit; 0<i; --i) std::cout <<(i<=pass.nLayer)? "1" : "0";
		std::cout <<std::endl;
	}

	return 0;

	uint32_t ppm = pass.addLayer();
	{
		std::cout <<"reference photon mapping [" <<ppm <<"]" <<std::endl;
		int nRay = 4;
		int nPhoton = 1000;
		int iteration = 10;
		float alpha = 0.6;
		float R0 = 1;
		RNG rng(0);
	
		std::vector<hitpoint> hits;
		hits.reserve(width*height*nRay);

		collectHitpoints(hits, pass.width, pass.height, nRay, scene, rng);
		for(hitpoint& hit : hits)hit.clear(R0);
		for(int i=0; i<iteration; i++){
			Tree photonmap = createPhotonmap(scene, nPhoton, rng);
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
	std::vector<std::vector<hitpoint>> hits(scene.cmpTargets.size());
	for(uint32_t targetID=0; targetID<scene.cmpTargets.size(); targetID++){
		int nRay = 16;
		int nDepth = 1;
		RNG rng(0);

		std::cout <<"collecting hitpoints for target component..." <<std::endl;
	
		hits[targetID].reserve(width*height*nRay);

		// uint32_t targetID = 0;
		collectHitpoints_target_exclusive(hits[targetID], targetID, nDepth,
			pass.width, pass.height, nRay, scene, rng);

		// save hitpoints
		// if(writeVector(hits, outDir + "/hit_2")) std::cout <<"hitpoints saved" <<std::endl;
	}


	// ppm
	// todo: takeover RNG state. currently hits are cleared before this iterations.
	for(uint32_t targetID=0; targetID<scene.cmpTargets.size(); targetID++){
		int nPhoton = 100000;
		int iteration = 10;
		float alpha = 0.6;
		float R0 = 0.5;

		std::cout <<"progressive photon mapping with " <<iteration <<" iterations..." <<std::endl;

		// uint32_t targetID = 0;

		for(hitpoint& hit : hits[targetID])hit.clear(R0);
		for(int i=0; i<iteration; i++){
			Tree photonmap = createPhotonmap_target(scene, nPhoton, targetID, rand);
			accumulateRadiance(hits[targetID], photonmap, scene, alpha);
		}

		// progressivePhotonMapping_target(hits, R0, iteration, nPhoton, alpha, scene, scene.cmpTargets[0], rand);
		std::string name = "hits" + std::to_string(targetID) + "_glass_64";
		writeVector(hits, outDir + "/" + name);
	}
	// readVector(hits, outDir + "/hit_1_100itr");


	// composition
	uint32_t composed = pass.addLayer();
	std::cout <<"composed [" <<composed <<"]" <<std::endl;
	for(int i=0; i<pass.length; i++)
		pass.data(composed)[i] = pass.data(nontarget)[i];

	for(uint32_t targetID=0; targetID<scene.cmpTargets.size(); targetID++){
		uint32_t target_raw = pass.addLayer();
		uint32_t target_txr = pass.addLayer();
		std::cout <<"raw target component [" <<target_raw <<"], ";
		std::cout <<"textured target component [" <<target_txr <<"], ";
		

		for(hitpoint hit : hits[targetID]){
			glm::vec3 tau = hit.tau/(float)(hit.iteration);

			double u = std::max(tau.x, std::max(tau.y, tau.z));
			u = pow(8*u, 1);
			pass.data(target_txr)[hit.pixel] += 0.4f*colormap_4(u) * hit.weight;
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