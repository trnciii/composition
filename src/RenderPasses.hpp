#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <fstream>
#include <cstring>

struct RenderPasses{
private:
	std::vector<glm::vec3> images;

public:

	const int width;
	const int height;
	const int length;
	uint32_t nLayer = 0;

	inline RenderPasses(int w, int h):width(w), height(h), length(w*h){
		images.resize(length*nLayer);
	}

	inline glm::vec3* data(uint32_t layer){return images.data() + (length * layer);}

	inline uint32_t addLayer(){
		nLayer++;
		images.resize(length*nLayer);
		return nLayer-1;
	}

	inline void setLayer_p(const uint32_t layer, glm::vec3* const i){
		if( layer < nLayer ){
			std::vector<glm::vec3>::iterator it(i);
			std::copy(it, it+length, this->data(layer));
		}
	}

	inline void setLayer(const uint32_t layer, std::vector<glm::vec3>::iterator begin){
		if(layer < nLayer){
			std::copy(begin, begin+length, this->data(layer));
		}
	}

	inline void setPixel(const uint32_t layer, const int i, const float x, const float y, const float z){
		images[layer*length + i] = glm::vec3(x, y, z);
	}

	inline void clear(){
		images.clear();
		nLayer = 0;
	}
};