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


	// render with path tracing
	std::cout <<"reference path tracing " <<std::flush;

	auto t0 = std::chrono::high_resolution_clock::now();

	Image im = PT(dim.x, dim.y, scene, 200, rng_per_pixel);

	auto t1 = std::chrono::high_resolution_clock::now();
	std::cout <<std::chrono::duration<double, std::milli>(t1-t0).count() <<" ms" <<std::endl;


	// save image
	std::string name = outDir + "/ref_pt.png";
	writeImage(im.data(), im.width(), im.height(), name);
	std::cout <<"image saved as " <<name <<std::endl;

	return 0;
}