#pragma once

#include <glm/glm.hpp>
#include <fstream>

struct RenderResult{
private:
	std::vector<glm::vec3> images;

public:
	enum Layer{
		EMISSION,
		DISTRIBUTION,
		TARGET,
		NONTARGET,
		COMPOSED
		// also update nLayer when you add layers
	};

	int width;
	int height;
	int length;
	int nLayer = 5;

	inline RenderResult(int w, int h):width(w), height(h), length(w*h){
		images.resize(w*h*nLayer);
	}

	inline glm::vec3* data(RenderResult::Layer layer){return images.data() + (length * layer);}

	inline std::string layerString(RenderResult::Layer layer){
		switch(layer){
			case RenderResult::Layer::COMPOSED: return "composed";
			case RenderResult::Layer::EMISSION: return "emission";
			case RenderResult::Layer::DISTRIBUTION: return "distribution";
			case RenderResult::Layer::TARGET: return "target";
			case RenderResult::Layer::NONTARGET: return "non-target";
		}
		return std::string();
	}

	bool write(const std::string& name);
	bool read(const std::string& name);
};


#ifdef IMPLEMENT_RENDER_RESULT

bool RenderResult::write(const std::string& name){
	std::ofstream out(name, std::ios::out | std::ios::binary);
	if (!out) return false;

	out.write(reinterpret_cast<const char*>(&width), sizeof(width));
	out.write(reinterpret_cast<const char*>(&height), sizeof(height));
	out.write(reinterpret_cast<const char*>(&nLayer), sizeof(nLayer));
	out.write(reinterpret_cast<const char*>(&images[0]), images.size() * sizeof(glm::vec3));
	out.close();

	return true;
}

bool RenderResult::read(const std::string& name) {
	images.clear();

	std::ifstream in(name, std::ios::in | std::ios::binary);
	if ( !in ) return false;
	
	in.read(reinterpret_cast<char*>(&width), sizeof(width));
	in.read(reinterpret_cast<char*>(&height), sizeof(height));
	in.read(reinterpret_cast<char*>(&nLayer), sizeof(nLayer));
	images.assign(width*height*nLayer, glm::vec3(0));
	in.read(reinterpret_cast<char*>(&images[0]), images.size() * sizeof(glm::vec3));
	in.close();

	length = width*height;

	return true;
}

#endif