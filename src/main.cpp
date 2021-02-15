#include "cmp.hpp"

#include <filesystem>
#include <cassert>
#include <iostream>
#include <bitset>
#include <omp.h>
#include <unordered_map>

#include "Image.hpp"
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
	glm::dvec2 dim(512, 512);
	std::unordered_map<std::string, Image> pass;

	if(scene.targetMaterials.size()==0){
		puts("no target");
		return 0;
	}

	{
		Image im(dim.x, dim.y);
		std::cout <<"reference path tracing" <<std::endl;

		RNG* rngForEveryPixel = new RNG[im.len()];
		for(int i=0; i<im.len(); i++)
			rngForEveryPixel[i] = RNG(i);

		pathTracing(im.data(),im.w, im.h, 200, scene, rngForEveryPixel);
		// writeLayer(pass, reference, outDir + "/reference");
		delete[] rngForEveryPixel;

		pass["pt"] = im;
	}

	{
		Image im(dim.x, dim.y);
		std::cout <<"reference photon mapping" <<std::endl;

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
		hits.reserve(im.len()*nRay);
		collectHitpoints(hits, im.w, im.h, nRay, scene, rngs[nThreads]);
		for(hitpoint& hit : hits)hit.clear(R0);

		for(int i=0; i<iteration; i++){
			Tree photonmap = createPhotonmap(scene, nPhoton, rngs.data(), nThreads);
			accumulateRadiance(hits, photonmap, scene, alpha);
		}

		for(hitpoint& hit : hits)
			im.pixels[hit.pixel] += hit.tau * hit.weight / (float)iteration;
		
		pass["ppm"] = im;
	}

	{
		Image im(dim.x, dim.y);
		std::cout <<"path tracing for non-target component" <<std::endl;

		RNG* rngForEveryPixel = new RNG[im.len()];
		for(int i=0; i<im.len(); i++)
			rngForEveryPixel[i] = RNG(i);

		pathTracing_notTarget(im.data(), im.w, im.h, 1000, scene, rngForEveryPixel);

		delete[] rngForEveryPixel;
		// writeLayer(pass["nt"], nontarget, outDir + "/nontarget");
		writeVector(im.pixels, outDir + "/" + "nt");
	}

	pass["nt"] = Image(dim.x, dim.y);
	readVector(pass["nt"].pixels, outDir+"/nt");


	// // collect hitpoints
	std::vector<std::vector<hitpoint>> hits(scene.targetMaterials.size());
	for(uint32_t i=0; i<scene.targetMaterials.size(); i++){
		int nRay = 512;
		int nDepth = 1;
		RNG rng(0);

		std::cout <<"collecting hitpoints on material " <<scene.targetMaterials[i] <<std::endl;
	
		hits[i].reserve(dim.x*dim.y*nRay);

		// uint32_t targetID = 0;
		collectHitpoints_target_exclusive(hits[i], scene.targetMaterials[i], nDepth,
			dim.x, dim.y, nRay, scene, rng);

		// save hitpoints
		// if(writeVector(hits, outDir + "/hit_2")) std::cout <<"hitpoints saved" <<std::endl;
	}


	// // ppm
	// // todo: takeover RNG state. currently hits are cleared before this iterations.
	{
		const int nPhoton = 1000;
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

	// // composition
	pass["composed"] = pass["nt"];
	for(uint32_t i=0; i<scene.targetMaterials.size(); i++){
		std::string raw = "mtl_" + std::to_string(scene.targetMaterials[i]) + "_raw";
		std::string remap = "mtl_" + std::to_string(scene.targetMaterials[i]) + "_remap";
		pass[raw] = Image(dim.x, dim.y);
		pass[remap] = Image(dim.x, dim.y);

		for(hitpoint hit : hits[i]){
			glm::vec3 tau = hit.tau/(float)(hit.iteration);

			double u = std::max(tau.x, std::max(tau.y, tau.z));
			u = pow(8*u, 1);

			pass[remap].pixels[hit.pixel] += tau*colormap_4(u) * hit.weight;
			pass[raw].pixels[hit.pixel] += tau*hit.weight;
		}

		for(int i=0; i<dim.x*dim.y; i++)
			pass["composed"].pixels[i] += pass[remap].pixels[i];
	}

	// save all image
	for(auto& [key, image] : pass){
		std::string name = outDir+"/"+key+".png";
		if(writeImage(image.data(), image.w, image.h, name))
			std::cout <<"saved " <<name <<std::endl;
	}

	return 0;
}