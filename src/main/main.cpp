#include "kernel.hpp"

#include <filesystem>
#include <cassert>
#include <iostream>
#include <bitset>
#include <omp.h>
#include <unordered_map>
#include <chrono>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Image.hpp"
#include "Random.hpp"
#include "file.hpp"
#include "toString.hpp"

glm::vec3 colormap_4(float u){
	if(u<0.25) return glm::vec3(0.1 , 0.1 , 0.85);
	if(u<0.5 ) return glm::vec3(0.1 , 0.85, 0.1 );
	if(u<0.75) return glm::vec3(0.85, 0.85, 0.1 );
	return glm::vec3(0.85, 0.1 , 0.1 );
}

unsigned char tonemap(double c){
	int c_out = 255*pow(c,(1/2.2)) +0.5;
	if(255 < c_out)c_out = 255;
	if(c_out < 0)c_out = 0;
	return c_out&0xff;
}

int writeImage(glm::vec3* color, int w, int h, const std::string& name){
	unsigned char *tone = new unsigned char[3*w*h];
	for(int i=0; i<w*h; i++){
		tone[3*i  ] = tonemap(color[i].x);
		tone[3*i+1] = tonemap(color[i].y);
		tone[3*i+2] = tonemap(color[i].z);
	}

	int result = stbi_write_png(name.c_str(), w, h, 3, tone, 3*w);

	delete[] tone;
	return result;
}

int main(void){

	std::string outDir("result");
	if(!( std::filesystem::exists(outDir) && std::filesystem::is_directory(outDir) )){
		std::cout <<"mkdir " <<outDir <<std::endl;
		assert(std::filesystem::create_directory(outDir));
	}

	glm::dvec2 dim(512, 512);
	std::unordered_map<std::string, Image> images;
	std::cout <<"image size: " <<dim.x <<", " <<dim.y <<std::endl;

	std::vector<RNG> rng_per_pixel = createRNGVector(dim.x*dim.y, 0);
	std::vector<RNG> rng_per_thread = createRNGVector(omp_get_max_threads(), dim.x*dim.y);

	Scene scene;
	createScene(&scene);
	std::cout <<str(scene) <<std::endl;

	if(scene.targetMaterials.size()==0){
		puts("no target");
		return 0;
	}


	{
		std::cout <<"nprr " <<std::flush;

		std::vector<std::function<glm::vec3(float)>> remaps(0);
		for(int i=0; i<scene.targetMaterials.size(); i++)
			remaps.push_back(colormap_4);

		images["nprr"] = nprr(dim.x, dim.y, scene, 200, rng_per_pixel, remaps);
	}


	std::cout <<std::endl;
	for(auto& [key, image] : images){
		std::string name = outDir+"/"+key+".png";
		if(writeImage(image.data(), image.width(), image.height(), name))
			std::cout <<"saved " <<name <<std::endl;
	}


	{
		std::cout <<"reference path tracing " <<std::flush;
		
		auto t0 = std::chrono::high_resolution_clock::now();

		Image im = PT(dim.x, dim.y, scene, 200, rng_per_pixel);

		auto t1 = std::chrono::high_resolution_clock::now();
		std::cout <<std::chrono::duration<double, std::milli>(t1-t0).count() <<" ms" <<std::endl;
		
		im.save(outDir+"/reference");
	}
	images["pt"] = Image(outDir+"/reference");


	{
		std::cout <<"reference photon mapping " <<std::flush;
		int nRay = 4;
		int nPhoton = 10000;
		int iteration = 100;
		float alpha = 0.6;
		float R0 = 1;

		auto t0 = std::chrono::high_resolution_clock::now();

		Image im = PPM(dim.x, dim.y, scene, nRay, nPhoton, iteration, alpha, R0, rng_per_thread);

		auto t1 = std::chrono::high_resolution_clock::now();
		std::cout <<std::chrono::duration<double, std::milli>(t1-t0).count() <<" ms" <<std::endl;

		images["ppm"] = im;
	}


	{
		std::cout <<"path tracing for non-target component " <<std::flush;

		auto t0 = std::chrono::high_resolution_clock::now();

		Image im = PT_notTarget(dim.x, dim.y, scene, 1000, rng_per_pixel);

		auto t1 = std::chrono::high_resolution_clock::now();
		std::cout <<std::chrono::duration<double, std::milli>(t1-t0).count() <<" ms" <<std::endl;

		im.save(outDir+"/nontarget");

		im = Image();
		images["nt"] = im.load(outDir+"/nontarget");
	}


	// // collect hitpoints
	std::vector<std::vector<hitpoint>> hits(scene.targetMaterials.size());
	for(uint32_t i=0; i<scene.targetMaterials.size(); i++){
		int nRay = 512;
		int nDepth = 1;
		float R0 = 1;
		RNG rng(0);
		uint32_t mtl = scene.targetMaterials[i];

		std::cout <<"collecting hitpoints on " <<scene.materials[mtl].name <<" " <<std::flush;
	
		hits[i].reserve(dim.x*dim.y*nRay/2);

		// uint32_t targetID = 0;

		auto t0 = std::chrono::high_resolution_clock::now();

		hits[i] = collectHitpoints_target_exclusive(mtl, nDepth, dim.x, dim.y, nRay, scene, rng_per_thread);
		for(hitpoint& hit : hits[i])hit.clear(R0);

		auto t1 = std::chrono::high_resolution_clock::now();
		std::cout <<std::chrono::duration<double, std::milli>(t1-t0).count() <<" ms" <<std::endl;

		// save hitpoints
		// if(writeVector(hits, outDir + "/hit_2")) std::cout <<"hitpoints saved" <<std::endl;
	}


	// // estimate exitance from hitpoints
	{
		const int nPhoton = 1000;
		const int iteration = 10;
		const float alpha = 0.6;
		const int spp = 1000;
		
		for(std::vector<hitpoint>& target : hits){
			// radiance_PT(target, scene, spp, rng_per_thread);
			radiance_PPM(target, scene, nPhoton, iteration, alpha, rng_per_thread);
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

			images[remap][hit.pixel] += tau*colormap_4(u) * hit.weight;
			images[raw][hit.pixel] += tau*hit.weight;
		}

		for(int i=0; i<dim.x*dim.y; i++)
			images["composed"][i] += images[remap][i];
	}

	// save all image
	std::cout <<std::endl;
	for(auto& [key, image] : images){
		std::string name = outDir+"/"+key+".png";
		if(writeImage(image.data(), image.width(), image.height(), name))
			std::cout <<"saved " <<name <<std::endl;
	}
	std::cout <<std::endl;

	return 0;
}