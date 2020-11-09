#define BOOST_PYTHON_STATIC_LIB
#define BOOST_DISABLE_PRAGMA_MESSAGE
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "cmp.hpp"
#include "RenderPasses.hpp"
#include "file.hpp"

std::vector<float> renderReference_wrap(
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

	std::vector<float> imagef(w*h*3);
	for(int i=0; i<w*h; i++){
		imagef[i  ] = result[i].x;
		imagef[i+1] = result[i].y;
		imagef[i+2] = result[i].z;
	}
	return imagef;
}


std::vector<float> renderNonTarget_wrap(
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

	std::vector<float> imagef(w*h*3);
	for(int i=0; i<w*h; i++){
		imagef[i  ] = result[i].x;
		imagef[i+1] = result[i].y;
		imagef[i+2] = result[i].z;
	}
	return imagef;
}

BOOST_PYTHON_MODULE(composition) {
	using namespace boost::python;

	class_<std::vector<float>>("vecf")
		.def(vector_indexing_suite<std::vector<float>>());

	class_<RenderPasses>("RenderPasses", init<int, int>())
		.def("addLayer", &RenderPasses::addLayer)
		.def("clear", &RenderPasses::clear);

	// class_<RNG>("RNG")
	// 	.def("seed", &RNG::seed)
	// 	.def("uniform", &RNG::uniform);

	class_<Scene>("Scene");
		// .def("add", &Scene::add)
		// .def("newMaterial", &Scene.newMaterial);

	def("createScene", createScene);
	def("renderReference", renderReference_wrap);
	def("renderNonTarget", renderNonTarget_wrap);
	def("writePasses", writePasses);
}