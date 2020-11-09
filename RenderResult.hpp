#pragma once

#include <glm/glm.hpp>
#include <fstream>

struct RenderResult{
private:
	std::vector<glm::vec3> images;

public:

	const int width;
	const int height;
	const int length;
	int nLayer = 0;

	inline RenderResult(int w, int h):width(w), height(h), length(w*h){
		images.resize(length*nLayer);
	}

	inline glm::vec3* data(int layer){return images.data() + (length * layer);}

	inline int addLayer(){
		nLayer++;
		images.resize(length*nLayer);
		return nLayer-1;
	}

	inline void clear(){
		images.clear();
		nLayer = 0;
	}
};