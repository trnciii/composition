#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <glm/glm.hpp>

struct Image{
private:
	int w;
	int h;
	std::vector<glm::vec3> pixels;

public:

	inline Image():w(0), h(0), pixels(0){}
	inline Image(int _w, int _h):w(_w), h(_h), pixels(_w*_h){}
	inline Image(const std::string n){load(n);}

	inline int width()const{return w;}
	inline int height()const{return h;}
	inline int size()const{return w*h;}
	inline glm::vec3* data(){return pixels.data();}

	inline glm::vec3& operator [](int i){return pixels[i];}
	inline glm::vec3& at(int i){return pixels[i];}
	inline glm::vec3& at(int x, int y){return pixels[y*w + x];}

	inline void resize(int _w, int _h){
		w = _w;
		h = _h;
		pixels.resize(w*h);
	}

	inline void setPixels(const float* src, size_t count){
		assert(count == 3*w*h);
		memcpy(pixels.data(), src, count*sizeof(float));
	}

	inline bool save(const std::string name)const{
		std::ofstream out(name, std::ios::out | std::ios::binary);
		if(!out) return false;
		if(pixels.size() != w*h) return false;

		out.write(reinterpret_cast<const char*>(&w), sizeof(w));
		out.write(reinterpret_cast<const char*>(&h), sizeof(h));
		out.write(reinterpret_cast<const char*>(pixels.data()), w*h * sizeof(glm::vec3));
		out.close();

		return true;
	}

	inline Image& load(const std::string name){
		pixels.clear();

		std::ifstream in(name, std::ios::in | std::ios::binary);
		if(!in) return *this = Image();

		in.read(reinterpret_cast<char*>(&w), sizeof(w));
		in.read(reinterpret_cast<char*>(&h), sizeof(h));
		
		pixels.resize(w*h);
		in.read(reinterpret_cast<char*>(pixels.data()), w*h * sizeof(glm::vec3));
		in.close();

		return *this;
	}
};