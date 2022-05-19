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

	std::vector<RNG> rng_per_pixel = createRNGVector(dim.x*dim.y, 0);

	Scene scene;
	createScene(&scene);
	std::cout <<str(scene) <<std::endl;

	if(scene.targetMaterials.size()==0){
		puts("no target");
		return 0;
	}


	// stylization based on path tracing

	// prepare color remapping function for each target.
	std::vector<std::function<glm::vec3(float)>> remaps(0);
	for(int i=0; i<scene.targetMaterials.size(); i++)
		remaps.push_back([](float u)->glm::vec3{
			// This is an peaky and ad-hoc part of this method.
			// The adjustment of u depends on the sample distribution.
			// typically, dividing by the light's emission shrinks u in [0, 1],
			u /= 30;
			u *= u;

			// map to color ramp
			if(u<0.25) return glm::vec3(0.1 , 0.1 , 0.85); // blue
			if(u<0.5 ) return glm::vec3(0.1 , 0.85, 0.1 ); // green
			if(u<0.75) return glm::vec3(0.85, 0.85, 0.1 ); // yellow
			return glm::vec3(0.85, 0.1 , 0.1 ); // red
		});

	Image im = nprr(dim.x, dim.y, scene, 1000, rng_per_pixel, remaps);


	// save result
	writeImage(im.data(), im.width(), im.height(), outDir + "/pt_based.png");

	return 0;
}