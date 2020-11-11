#define BOOST_PYTHON_STATIC_LIB
#define BOOST_DISABLE_PRAGMA_MESSAGE
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "cmp.hpp"
#include "RenderPasses.hpp"
#include "file.hpp"
#include "data.hpp"


inline std::vector<float> getBlenderImage(RenderPasses passes, const uint32_t layer){
	const glm::vec3* const v = passes.data(layer);

	std::vector<float> f(passes.length*4);
	for(int y=0; y<passes.height; y++){
		for(int x=0; x<passes.width; x++){
			int i_s = y*passes.width + x;
			int i_d = (passes.height -y-1)*passes.width + x;

			f[4*i_d  ] = v[i_s].x;
			f[4*i_d+1] = v[i_s].y;
			f[4*i_d+2] = v[i_s].z;
			f[4*i_d+3] = 1.0f;
		}
	}
	return f;
}

void renderReference_wrap(
	RenderPasses& passes, const int layer, const int spp, const Scene& scene)
{
	glm::vec3* const result = passes.data(layer);
	const int w = passes.width;
	const int h = passes.height;

	RNG* rngForEveryPixel = new RNG[w*h];
	for(int i=0; i<w*h; i++)
		rngForEveryPixel[i] = RNG(i);

	renderReference(result, w, h, spp, scene, rngForEveryPixel);

	delete[] rngForEveryPixel;
}

void renderNonTarget_wrap(
	RenderPasses& passes, const int layer, const int spp, const Scene& scene)
{
	glm::vec3* const result = passes.data(layer);
	const int w = passes.width;
	const int h = passes.height;

	RNG* rngForEveryPixel = new RNG[w*h];
	for(int i=0; i<w*h; i++)
		rngForEveryPixel[i] = RNG(i);

	renderNonTarget(result, w, h, spp, scene, rngForEveryPixel);

	delete[] rngForEveryPixel;
}

BOOST_PYTHON_MODULE(composition) {
	using namespace boost::python;

	class_<std::vector<float>>("vF")
		.def(vector_indexing_suite<std::vector<float>>());

	class_<RenderPasses>("RenderPasses", init<int, int>())
		.def_readonly("width", &RenderPasses::width)
		.def_readonly("height", &RenderPasses::height)
		.def_readonly("nLayers", &RenderPasses::nLayer)
		// .def("data", &RenderPasses::data)
		.def("addLayer", &RenderPasses::addLayer)
		.def("clear", &RenderPasses::clear);

	class_<hitpoint>("hitpoint");
	// class_<std::vector<hitpoint>>("vHit")
		// .def(vector_indexing_suite<std::vector<hitpoint>>());

	// class_<RNG>("RNG")
	// 	.def("seed", &RNG::seed)
	// 	.def("uniform", &RNG::uniform);

	class_<Scene>("Scene");
		// .def("add", &Scene::add)
		// .def("newMaterial", &Scene.newMaterial);


	// this - data
	def("getImage", getBlenderImage);

	// this - main functions
	def("createScene", createScene);
	def("renderReference", renderReference_wrap);
	def("renderNonTarget", renderNonTarget_wrap);

	// file.hpp
	def("writeAllPasses", writeAllPasses);
}