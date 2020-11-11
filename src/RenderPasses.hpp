#pragma once

#include <glm/glm.hpp>
#include <fstream>

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

	inline void clear(){
		images.clear();
		nLayer = 0;
	}
};