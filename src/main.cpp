#include "cmp.hpp"

#include <filesystem>
#include <cassert>
#include <iostream>
#include <bitset>

#include "RenderPasses.hpp"
#include "Random.hpp"
#include "print.hpp"
#include "file.hpp"

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

	// render parameters
	int nIteration = 10000;
	int outInterval = 100;

	int nPhoton = 10000;
	float initialRadius = 1;
	int nRay = 16;
	float alpha = 0.7;
	int spp_pt = 1000;

	if(scene.cmpTargets.size()==0){
		puts("no target");
		return 0;
	}
	uint32_t target = scene.cmpTargets[0];


	// uint32_t reference = passes.addLayer();
	// {
	// 	std::cout <<"path tracing for reference..." <<std::endl;

	// 	RNG* rngForEveryPixel = new RNG[width*height];
	// 	for(int i=0; i<width*height; i++)
	// 		rngForEveryPixel[i] = RNG(i);

	// 	renderReference(passes.data(reference), width, height, 100, scene, rngForEveryPixel);

	// 	writeLayer(passes, reference, outDir + "/reference");

	// 	delete[] rngForEveryPixel;
	// }

	// uint32_t non_target = passes.addLayer();
	// {
	// 	std::cout <<"path tracing for non-target component..." <<std::endl;

	// 	RNG* rngForEveryPixel = new RNG[width*height];
	// 	for(int i=0; i<width*height; i++)
	// 		rngForEveryPixel[i] = RNG(i);

	// 	renderNonTarget(passes.data(non_target), width, height, 100, scene, rngForEveryPixel);

	// 	delete[] rngForEveryPixel;
	// }

	uint32_t reference = passes.addLayer();
	loadLayer(passes, reference, outDir + "/reference");
	
	uint32_t non_target = passes.addLayer();
	loadLayer(passes, non_target, outDir + "/nontarget");
	

	// uint32_t distribution0 = passes.addLayer();
	// {
	// 	std::cout <<"collecting hitpoints for target component..." <<std::endl;

	// 	std::vector<hitpoint> hits;
	// 	hits.reserve(width*height*nRay);
	// 	RNG rng(0);

	// 	collectHitpoints(hits, passes.width, passes.height, nRay,
	// 		initialRadius, scene, target, rng);

	// 	// compose distriburion image
	// 	std::vector<glm::vec3> dist(width*height);
	// 	for(auto hit : hits){
	// 		dist[hit.pixel] += hit.weight;
	// 	}

	// 	passes.set(distribution0, dist.data());
			
	// 	// save hitpoints
	// 	if(writeVector(hits, outDir + "/hit"))std::cout <<"hitpoints saved" <<std::endl;
	// }


	// read hitpoints from a file
	uint32_t distribution1 = passes.addLayer();
	{
		std::vector<hitpoint> hits;
		if(readVector(hits, outDir + "/hit"))std::cout <<"hitpoints load" <<std::endl;
		
		std::vector<glm::vec3> im_d(width*height);
		for(auto hit : hits) im_d[hit.pixel] += hit.weight;
		
		passes.setLayer(distribution1, im_d.begin());
	}

	// ppm
	{
		Tree photonmap = createPhotonmap(scene, nPhoton, target, &rand);
		accumulateRadiance(hit_target, photonmap, scene, alpha);
	}


	// composition
	{		
		for(hitpoint hit : hit_target){
			vec3 tau = hit.tau/iteration;
			double u = max(tau);
			u = pow(8*u, 1);
			// result.data(RenderResult::Layer::TARGET)[hit.pixel] += 0.4*colormap_4(u) * hit.weight;
			result.data(RenderResult::Layer::TARGET)[hit.pixel] += tau*hit.weight;
		}

		for(int i=0; i<result.length; i++)
			result.data(RenderResult::Layer::COMPOSED)[i] = result.data(RenderResult::Layer::EMISSION)[i] + result.data(RenderResult::Layer::TARGET)[i] + result.data(RenderResult::Layer::NONTARGET)[i]; 
	}

	// std::cout <<"progressive estimation pass for target component..." <<std::endl;
	// printBr();
	// for(int iteration=1; iteration<=nIteration; iteration++){
	// 	Tree photonmap = createPhotonmap(scene, nPhoton, target, &rand);
	// 	accumulateRadiance(hit_target, photonmap, scene, alpha);

	// 	if(iteration%outInterval == 0 || iteration == nIteration){
	// 		std::cout <<"composing an image" <<std::endl;
	// 		std::cout <<"itr = " <<iteration <<std::endl;

	// 		composeImage(result, iteration, hit_target);
			
	// 		char out_target[64];
	// 		sprintf(out_target, "targat_%04d.png", iteration);

	// 		char out_comp[64];
	// 		sprintf(out_comp, "composed_%04d.png", iteration);

	// 		saveResult(result, RenderResult::Layer::TARGET, outDir_p, out_target);
	// 		saveResult(result, RenderResult::Layer::COMPOSED, outDir_p, out_comp);
	// 		saveResult(result, RenderResult::Layer::TARGET, outDir);
	// 		saveResult(result, RenderResult::Layer::COMPOSED, outDir);
		
	// 		printRule();
	// 	}
	// }

	// save
	{
		const int digit = 8;
		std::cout <<"pass output: " <<std::bitset<digit>(writeAllPasses(passes, outDir)) <<" / ";
		for(int i=digit; 0<i; --i) std::cout <<(i<=passes.nLayer)? "1" : "0";
		std::cout <<std::endl;
	}	

	return 0;
}