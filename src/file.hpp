#pragma once

#include <string>
#include <fstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include "RenderPass.hpp"

template<typename T>
bool writeVector(const std::vector<T>& v, const std::string& name) {
	std::ofstream out(name, std::ios::out | std::ios::binary);
	if (!out) return false;

	int count = v.size();
	out.write(reinterpret_cast<const char*>(&count), sizeof(count));
	out.write(reinterpret_cast<const char*>(&v[0]), v.size() * sizeof(T));
	out.close();

	return true;
}

template<typename T>
bool readVector(std::vector<T>& v, const std::string& name) {
	v.clear();
	int count;

	std::ifstream in(name, std::ios::in | std::ios::binary);
	if ( !in ) return false;
	
	in.read(reinterpret_cast<char*>(&count), sizeof(count));
	v.resize(count);
	in.read(reinterpret_cast<char*>(&v[0]), count * sizeof(T));
	in.close();

	return true;
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

// todo: size check
bool readPixels(Image& im, const std::string& path){
	return readVector(im.pixels, path);
}

bool writePixels(Image& im, const std::string& path){
	return writeVector(im.pixels, path);
}
