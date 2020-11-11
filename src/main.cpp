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


	uint32_t reference = passes.addLayer();
	{
		std::cout <<"path tracing for reference..." <<std::endl;

		RNG* rngForEveryPixel = new RNG[width*height];
		for(int i=0; i<width*height; i++)
			rngForEveryPixel[i] = RNG(i);

		renderReference(passes.data(reference), width, height, 100, scene, rngForEveryPixel);

		delete[] rngForEveryPixel;
	}


	uint32_t non_target = passes.addLayer();
	{
		std::cout <<"path tracing for non-target component..." <<std::endl;

		RNG* rngForEveryPixel = new RNG[width*height];
		for(int i=0; i<width*height; i++)
			rngForEveryPixel[i] = RNG(i);

		renderNonTarget(passes.data(non_target), width, height, 100, scene, rngForEveryPixel);

		delete[] rngForEveryPixel;
	}
	

	uint32_t distribution0 = passes.addLayer();
	{
		std::cout <<"collecting hitpoints for target component..." <<std::endl;

		std::vector<hitpoint> hits;
		hits.reserve(width*height*nRay);
		RNG rng(0);

		collectHitpoints(hits, passes.width, passes.height, nRay,
			initialRadius, scene, target, rng);

		// compose distriburion image
		std::vector<glm::vec3> dist(width*height);
		for(auto hit : hits){
			dist[hit.pixel] += hit.weight;
		}

		for(int i=0; i<passes.length; i++){
			passes.set(distribution0, i, dist[i].x, dist[i].y, dist[i].z);
		}
			
		// save hitpoints
		if(writeVector(hits, outDir + "/hit"))std::cout <<"hitpoints saved" <<std::endl;
	}


	// read test
	//
	// uint32_t distribution1 = passes.addLayer();
	// {
	// 	std::vector<hitpoint> hits;
	// 	if(readVector(hits, outDir + "/hit"))std::cout <<"hitpoints load" <<std::endl;
	// 	for(auto hit : hits) passes.data(distribution1)[hit.pixel] += hit.weight;
	// 	if(writeImage(passes.data(distribution1),width, height, (outDir + "/distribution1.png").data()) == 1)
	// 		std::cout <<" distribution1 saved" <<std::endl;
	// 	else std::cout <<"failed to save image" <<std::endl;
	// }


	// std::cout <<"progressive estimation pass for target component..." <<std::endl;
	// printBr();
	// for(int iteration=1; iteration<=nIteration; iteration++){
	// 	Tree photonmap = createPhotonmap(scene, nPhoton, aggregationTarget, &rand);
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

	std::cout <<"pass output: " <<std::bitset<8>(writeAllPasses(passes, outDir)) <<std::endl;

	return 0;
}