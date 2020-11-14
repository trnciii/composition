#include "cmp.hpp"

#include <filesystem>
#include <cassert>
#include <iostream>
#include <bitset>

#include "RenderPasses.hpp"
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

	std::string outDir_p = outDir + "/progress";
	if(!( std::filesystem::exists(outDir_p) && std::filesystem::is_directory(outDir_p) )){
		std::cout <<"mkdir " <<outDir_p <<std::endl;
		printBr();
		assert(std::filesystem::create_directory(outDir_p));
	}
	
	Scene scene;
	createScene(&scene);
	print(scene);

	RNG rand;
	int width = 512;
	int height = 512;
	RenderPasses passes(width, height);

	if(scene.cmpTargets.size()==0){
		puts("no target");
		return 0;
	}
	uint32_t targetObject = scene.cmpTargets[0];


	// render path traced reference
	// uint32_t reference = passes.addLayer();
	// {
	// 	std::cout <<"path tracing for reference..." <<std::endl;

	// 	RNG* rngForEveryPixel = new RNG[width*height];
	// 	for(int i=0; i<width*height; i++)
	// 		rngForEveryPixel[i] = RNG(i);

	// 	renderReference(passes.data(reference), width, height, 10000, scene, rngForEveryPixel);

	// 	writeLayer(passes, reference, outDir + "/reference");
	// 	delete[] rngForEveryPixel;
	// }
	// loadLayer(passes, reference, outDir + "/reference");


	// render non target component with pt
	uint32_t nontarget = passes.addLayer();
	// {
	// 	std::cout <<"path tracing for non-target component..." <<std::endl;

	// 	RNG* rngForEveryPixel = new RNG[width*height];
	// 	for(int i=0; i<width*height; i++)
	// 		rngForEveryPixel[i] = RNG(i);

	// 	renderNonTarget(passes.data(nontarget), width, height, 10000, scene, rngForEveryPixel);

	// 	delete[] rngForEveryPixel;
	// 	writeLayer(passes, nontarget, outDir + "/nontarget");
	// }
	loadLayer(passes, nontarget, outDir + "/nontarget");
	

	// collect hitpoints
	std::vector<hitpoint> hits;
	// {
	// 	int nRay = 128;

	// 	std::cout <<"collecting hitpoints for target component..." <<std::endl;
	
	// 	hits.reserve(width*height*nRay);
	// 	RNG rng(0);

	// 	collectHitpoints(hits, 2, passes.width, passes.height, nRay,
	// 		0, scene, targetObject, rng);
			
	// 	// save hitpoints
	// 	if(writeVector(hits, outDir + "/hit_2"))std::cout <<"hitpoints saved" <<std::endl;
	// }
	readVector(hits, outDir + "/hit_1");

	// visualize weight of hits
	uint32_t weights = passes.addLayer();
	{		
		std::vector<glm::vec3> im_d(passes.length);
		for(auto hit : hits) im_d[hit.pixel] += hit.weight;
		passes.setLayer(weights, im_d.begin());
	}

	// ppm
	// todo: takeover RNG state. currently hits are cleared before this iterations.
	{
		int nPhoton = 10000;
		int iteration = 100;
		float alpha = 0.7;
		float R0 = 1;

		std::cout <<"progressive photon mapping with " <<iteration <<" iterations..." <<std::endl;

		progressivePhotonMapping(hits, R0, iteration, nPhoton, alpha, scene, targetObject, rand);
		writeVector(hits, outDir + "/hit_1_100itr");
	}
	readVector(hits, outDir + "/hit_1_100itr");


	// composition
	uint32_t target = passes.addLayer();
	uint32_t composed = passes.addLayer();
	{
		std::cout <<"compositing image..." <<std::endl;

		for(hitpoint hit : hits){
			glm::vec3 tau = hit.tau/(float)(hit.iteration);

			double u = std::max(tau.x, std::max(tau.y, tau.z));
			u = pow(8*u, 1);
			// passes.data(target)[hit.pixel] += 0.4f*colormap_4(u) * hit.weight;
			passes.data(target)[hit.pixel] += tau*hit.weight;
		}

		for(int i=0; i<passes.length; i++)
			passes.data(composed)[i] = passes.data(target)[i] + passes.data(nontarget)[i];
	}


	// save all image
	{
		const int digit = 8;
		std::cout <<"pass output: " <<std::bitset<digit>(writeAllPasses(passes, outDir)) <<" / ";
		for(int i=digit; 0<i; --i) std::cout <<(i<=passes.nLayer)? "1" : "0";
		std::cout <<std::endl;
	}	

	return 0;
}