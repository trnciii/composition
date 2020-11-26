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
	uint32_t targetObject = scene.cmpTargets[0];


	// render path traced reference
	// uint32_t reference = pass.addLayer();
	// loadLayer(pass, reference, outDir + "/reference");
	// {
	// 	std::cout <<"path tracing for reference [" <<reference <<"]" <<std::endl;

	// 	RNG* rngForEveryPixel = new RNG[width*height];
	// 	for(int i=0; i<width*height; i++)
	// 		rngForEveryPixel[i] = RNG(i);

	// 	renderReference(pass.data(reference), width, height, 5000, scene, rngForEveryPixel);
	// 	writeLayer(pass, reference, outDir + "/reference");
	// 	delete[] rngForEveryPixel;
	// }

	uint32_t ppm = pass.addLayer();
	{
		int nRay = 4;
		int nPhoton = 100000;
		int iteration = 10;
		float alpha = 0.6;
		float R0 = 0.5;
		RNG rng(0);
	
		std::vector<hitpoint> hits;
		hits.reserve(width*height*nRay);

		collectHitpoints_all(hits, pass.width, pass.height, nRay, 0, scene, rng);
		for(hitpoint& hit : hits)hit.clear(R0);
		for(int i=0; i<iteration; i++){
			Tree photonmap = createPhotonmap_all(scene, nPhoton, rng);
			accumulateRadiance(hits, photonmap, scene, alpha);
		}

		glm::vec3* image = pass.data(ppm);
		for(hitpoint& hit : hits){
			image[hit.pixel] += hit.tau * hit.weight / (float)iteration;
		}
	}

	// render non target component with pt
	const uint32_t nontarget = pass.addLayer();
	loadLayer(pass, nontarget, outDir + "/nontarget");
	// {
	// 	std::cout <<"path tracing for non-target component [" <<nontarget <<"]" <<std::endl;

	// 	RNG* rngForEveryPixel = new RNG[width*height];
	// 	for(int i=0; i<width*height; i++)
	// 		rngForEveryPixel[i] = RNG(i);

	// 	renderNonTarget(pass.data(nontarget), width, height, 1000, scene, rngForEveryPixel);

	// 	delete[] rngForEveryPixel;
	// 	writeLayer(pass, nontarget, outDir + "/nontarget");
	// }
	

	// collect hitpoints
	std::vector<hitpoint> hits;
	{
		int nRay = 16;
		int nDepth = 1;
		RNG rng(0);

		std::cout <<"collecting hitpoints for target component..." <<std::endl;
	
		hits.reserve(width*height*nRay);

		collectHitpoints_target(hits, nDepth, pass.width, pass.height, nRay,
			0, scene, targetObject, rng);

		// save hitpoints
		if(writeVector(hits, outDir + "/hit_2")) std::cout <<"hitpoints saved" <<std::endl;
	}
	// readVector(hits, outDir + "/hit_1");

	// visualize weight of hits
	uint32_t weights = pass.addLayer();
	std::cout <<"visualization of weights [" <<weights <<"]" <<std::endl;
	{	
		std::vector<glm::vec3> im_d(pass.length);
		for(hitpoint& hit : hits) im_d[hit.pixel] += hit.weight;
		pass.setLayer(weights, im_d.begin());
	}

	// ppm
	// todo: takeover RNG state. currently hits are cleared before this iterations.
	{
		int nPhoton = 100000;
		int iteration = 100;
		float alpha = 0.6;
		float R0 = 0.5;

		std::cout <<"progressive photon mapping with " <<iteration <<" iterations..." <<std::endl;

		for(hitpoint& hit : hits)hit.clear(R0);
		for(int i=0; i<iteration; i++){
			Tree photonmap = createPhotonmap_target(scene, nPhoton, targetObject, rand);
			accumulateRadiance(hits, photonmap, scene, alpha);
		}

		// progressivePhotonMapping_target(hits, R0, iteration, nPhoton, alpha, scene, targetObject, rand);
		std::cout <<"C" <<std::endl;
		// writeVector(hits, outDir + "/hit_1_100itr");
	}
	// readVector(hits, outDir + "/hit_1_100itr");


	// composition
	uint32_t target_raw = pass.addLayer();
	uint32_t target_txr = pass.addLayer();
	uint32_t composed = pass.addLayer();
	{
		std::cout <<"compositing image.";
		std::cout <<"raw target component [" <<target_raw <<"], ";
		std::cout <<"textured target component [" <<target_txr <<"], ";
		std::cout <<"composed [" <<composed <<"]" <<std::endl;

		for(hitpoint hit : hits){
			glm::vec3 tau = hit.tau/(float)(hit.iteration);

			double u = std::max(tau.x, std::max(tau.y, tau.z));
			u = pow(8*u, 1);
			pass.data(target_txr)[hit.pixel] += 0.4f*colormap_4(u) * hit.weight;
			pass.data(target_raw)[hit.pixel] += tau*hit.weight;
		}

		for(int i=0; i<pass.length; i++)
			pass.data(composed)[i] = pass.data(target_txr)[i] + pass.data(nontarget)[i];
	}


	// save all image
	{
		const int digit = 8;
		std::cout <<"pass output: " <<std::bitset<digit>(writeAllPasses(pass, outDir)) <<" / ";
		for(int i=digit; 0<i; --i) std::cout <<(i<=pass.nLayer)? "1" : "0";
		std::cout <<std::endl;
	}	

	return 0;
}