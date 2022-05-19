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

	glm::dvec2 dim(512, 512); // image dimension
	std::unordered_map<std::string, Image> images; // layers

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
		std::cout <<"path tracing for non-target component " <<std::flush;

		auto t0 = std::chrono::high_resolution_clock::now();

		images["nt"] = PT_notTarget(dim.x, dim.y, scene, 1000, rng_per_pixel);

		auto t1 = std::chrono::high_resolution_clock::now();
		std::cout <<std::chrono::duration<double, std::milli>(t1-t0).count() <<" ms" <<std::endl;
	}


	// collect hitpoints
	std::vector<std::vector<hitpoint>> hits(scene.targetMaterials.size());
	for(uint32_t i=0; i<scene.targetMaterials.size(); i++){
		int nRay = 100;
		int nDepth = 1;
		float R0 = 1;
		RNG rng(0);
		uint32_t mtl = scene.targetMaterials[i];

		std::cout <<"collecting hitpoints on " <<scene.materials[mtl].name <<" " <<std::flush;
	
		hits[i].reserve(dim.x*dim.y*nRay/2);

		auto t0 = std::chrono::high_resolution_clock::now();

		hits[i] = collectHitpoints_target_exclusive(mtl, nDepth, dim.x, dim.y, nRay, scene, rng_per_thread);
		for(hitpoint& hit : hits[i])hit.clear(R0);

		auto t1 = std::chrono::high_resolution_clock::now();
		std::cout <<std::chrono::duration<double, std::milli>(t1-t0).count() <<" ms" <<std::endl;
	}



	{ // estimate radiance from each hitpoint
		// this part is quite heavy especially with caustic materials.
		// adjust the below parameters according to PPM paper.
		// Once radiance is calculated, we can save the hitpoint vector and omit this part.

		const int nPhoton = 10000;
		const int iteration = 1000;
		const float alpha = 0.6;
		const int spp = 1000;

		for(uint32_t i=0; i<scene.targetMaterials.size(); i++){
			// radiance_PT(hits[i], scene, spp, rng_per_thread); // using path tracing for radiance estimation is also ok.
			radiance_PPM(hits[i], scene, nPhoton, iteration, alpha, rng_per_thread);

			writeVector(hits[i], outDir + "hitpoint_" + scene.materials[scene.targetMaterials[i]].name);
		}
	}

	// load the saved hitpoints
	// for(int i=0; i<hits.size(); i++)
		// readVector(hits[i], outDir + "hitpoint_" + scene.materials[scene.targetMaterials[i]].name);


	// composition
	images["composed"] = images["nt"];
	for(uint32_t i=0; i<scene.targetMaterials.size(); i++){
		std::string raw = "mtl_" + std::to_string(scene.targetMaterials[i]) + "_raw";
		images[raw] = Image(dim.x, dim.y); // layer without stylization

		std::string remap = "mtl_" + std::to_string(scene.targetMaterials[i]) + "_remap";
		images[remap] = Image(dim.x, dim.y); // layer stylized

		// accumulate pixel color
		for(hitpoint& hit : hits[i]){
			glm::vec3 tau = hit.tau/(float)(hit.iteration); // radiance per hitpoint

			images[raw][hit.pixel] += tau*hit.weight; // accum without stylization(not used).


			// apply any stylization and accum to the pixel.
			// below is color ramp sampled with max(R, G, B)

			// u is the texture coordinate corresponding to the radiance tau,
			double u = std::max(tau.x, std::max(tau.y, tau.z));
			u = 2*pow(u, 0.7); // and any mapping is ok.

			// new radiance is the texture color * tau (white radiance).
			glm::vec3 color = tau; // but also try to start with glm::vec3(1,1,1); for different appearance.
			if(u<0.25) color *= glm::vec3(0.1 , 0.1 , 0.85); // blue
			else if(u<0.5 ) color *= glm::vec3(0.1 , 0.85, 0.1 ); // green
			else if(u<0.75) color *= glm::vec3(0.85, 0.85, 0.1 ); // yellow
			else color *= glm::vec3(0.85, 0.1 , 0.1); // red

			images[remap][hit.pixel] += color * hit.weight;
		}

		for(int i=0; i<dim.x*dim.y; i++)
			images["composed"][i] += images[remap][i];
	}

	// save all images
	std::cout <<std::endl;
	for(auto& [key, image] : images){
		std::string name = outDir+"/"+key+".png";
		if(writeImage(image.data(), image.width(), image.height(), name))
			std::cout <<"saved " <<name <<std::endl;
	}
	std::cout <<std::endl;

	return 0;
}