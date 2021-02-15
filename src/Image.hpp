#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <glm/glm.hpp>

struct Image{
	int w;
	int h;
	std::vector<glm::vec3> pixels;

	inline Image():w(0), h(0), pixels(0){}
	inline Image(int _w, int _h):w(_w), h(_h), pixels(_w*_h){}

	inline int len(){return w*h;}
	inline glm::vec3* data(){return pixels.data();}
};