#pragma once

#include <string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include "RenderPasses.hpp"

unsigned char tonemap(double c){
	int c_out = 255*pow(c,(1/2.2)) +0.5;
	if(255 < c_out)c_out = 255;
	if(c_out < 0)c_out = 0;
	return c_out&0xff;
}

int writeImage(glm::vec3* color, int w, int h, const char* name){
	unsigned char *tone = new unsigned char[3*w*h];
	for(int i=0; i<w*h; i++){
		tone[3*i  ] = tonemap(color[i].x);
		tone[3*i+1] = tonemap(color[i].y);
		tone[3*i+2] = tonemap(color[i].z);
	}

	int result = stbi_write_png(name, w, h, 3, tone, 3*w);

	delete[] tone;
	return result;
}

int writePasses(RenderPasses& passes, const std::string& dir){
	int success = 0;
	for(int n=0; n<passes.nLayer; n++){
		std::string name = std::to_string(n) + ".png";
		if(writeImage(passes.data(n), passes.width, passes.height, (dir + "/" + name).data()) == 1)
			success |= 1<<n;
	}
	return success;
}