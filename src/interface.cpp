#define BOOST_PYTHON_STATIC_LIB
#define BOOST_DISABLE_PRAGMA_MESSAGE
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <string>

#include "cmp.hpp"
#include "RenderPass.hpp"
#include "file.hpp"
#include "data.hpp"

inline std::vector<float> getBlenderImage(RenderPass pass, const uint32_t layer){
	// pass image in linear? space
	const glm::vec3* const v = pass.data(layer);

	std::vector<float> f(pass.length*4);
	for(int y=0; y<pass.height; y++){
		for(int x=0; x<pass.width; x++){
			int i_s = y*pass.width + x;
			int i_d = (pass.height -y-1)*pass.width + x;

			f[4*i_d  ] = v[i_s].x;
			f[4*i_d+1] = v[i_s].y;
			f[4*i_d+2] = v[i_s].z;
			f[4*i_d+3] = 1.0f;
		}
	}
	return f;
}

void renderReference_wrap(
	RenderPass& pass, const int layer, const int spp, const Scene& scene)
{
	glm::vec3* const result = pass.data(layer);
	const int w = pass.width;
	const int h = pass.height;

	RNG* rngForEveryPixel = new RNG[w*h];
	for(int i=0; i<w*h; i++)
		rngForEveryPixel[i] = RNG(i);

	renderReference(result, w, h, spp, scene, rngForEveryPixel);

	delete[] rngForEveryPixel;
}

void renderNonTarget_wrap(
	RenderPass& pass, const int layer, const int spp, const Scene& scene)
{
	glm::vec3* const result = pass.data(layer);
	const int w = pass.width;
	const int h = pass.height;

	RNG* rngForEveryPixel = new RNG[w*h];
	for(int i=0; i<w*h; i++)
		rngForEveryPixel[i] = RNG(i);

	renderNonTarget(result, w, h, spp, scene, rngForEveryPixel);

	delete[] rngForEveryPixel;
}

class hitpoints_wrap{
public:
	std::vector<hitpoint> hits;

	hitpoints_wrap():hits(0){}
	hitpoints_wrap(std::vector<hitpoint> h):hits(h){}

	uint32_t size(){return hits.size();}
	hitpoint element(uint32_t i){return hits[i];}

	std::string save(const std::string& s){
		std::string result;
		if(writeVector(hits, s))
			result = std::to_string(hits.size()) + "hitpoints saved.";
		else
			result = "failed to save hitpoints";
		std::cout <<result <<std::endl;
		return result;
	}

	std::string load(const std::string& s){
		std::string result;
		if(readVector(hits, s))
			result = std::to_string(hits.size()) + " hitpoints loaded";
		else
			result = "failed to load hitpoints";
		std::cout <<result <<std::endl;
		return result;
	}
};

hitpoints_wrap collectHitpoints_all_wrap(const int depth, const int w, const int h, const int nRay,
	const Scene& scene, const uint32_t target)
// todo: passing rng state
{
	std::vector<hitpoint> hits;
	hits.reserve(w*h*nRay);
	RNG rng(0);

	collectHitpoints_target(hits, target, depth, w, h, nRay, scene, rng);

	return hitpoints_wrap(hits);
}

hitpoints_wrap collectHitpoints_one_wrap(const int depth, const int w, const int h, const int nRay,
	const Scene& scene, const uint32_t target)
// todo: passing rng state
{
	std::vector<hitpoint> hits;
	hits.reserve(w*h*nRay);
	RNG rng(0);

	collectHitpoints_target_one(hits, target, depth, w, h, nRay, scene, rng);

	return hitpoints_wrap(hits);
}

void progressivePhotonMapping_all(std::vector<hitpoint>& hits, 
	const float R0, const int iteration, const int nPhoton, const float alpha,
	const Scene& scene)
{
	RNG rng;
	for(hitpoint& hit : hits)hit.clear(R0);
	for(int i=0; i<iteration; i++){
		Tree photonmap = createPhotonmap_all(scene, nPhoton, rng);
		accumulateRadiance(hits, photonmap, scene, alpha);
	}
}

void progressivePhotonMapping_target(hitpoints_wrap& hw,
	const float R0, const int iteration, const int nPhoton, const float alpha,
	const Scene& scene, const uint32_t target)
{
	RNG rng;
	std::vector<hitpoint>& hits = hw.hits;
	for(hitpoint& hit : hits)hit.clear(R0);
	for(int i=0; i<iteration; i++){
		Tree photonmap = createPhotonmap_target(scene, nPhoton, target, rng);
		accumulateRadiance(hits, photonmap, scene, alpha);
	}
}

BOOST_PYTHON_MODULE(composition) {
	using namespace boost::python;

	class_<glm::vec3>("vec3")
		.def_readonly("x", &glm::vec3::x)
		.def_readonly("y", &glm::vec3::y)
		.def_readonly("z", &glm::vec3::z);

	class_<std::vector<float>>("vec3s")
		.def(vector_indexing_suite<std::vector<float>>());

	class_<RenderPass>("RenderPass", init<int, int>())
		.def_readonly("width", &RenderPass::width)
		.def_readonly("height", &RenderPass::height)
		.def_readonly("nLayers", &RenderPass::nLayer)
		.def("addLayer", &RenderPass::addLayer)
		.def("clear", &RenderPass::clear)
		.def("set", &RenderPass::setPixel);

	class_<hitpoint>("hitpoint")
		.def_readonly("p", &hitpoint::p)
		.def_readonly("n", &hitpoint::n)
		.def_readonly("wo", &hitpoint::wo)
		.def_readonly("mtlID", &hitpoint::mtlID)
		.def_readonly("pixel", &hitpoint::pixel)
		.def_readonly("R", &hitpoint::R)
		.def_readonly("N", &hitpoint::N)
		.def_readonly("tau", &hitpoint::tau)
		.def_readonly("weight", &hitpoint::weight)
		.def_readonly("iteration", &hitpoint::iteration)
		.def_readonly("depth", &hitpoint::depth);

	class_<hitpoints_wrap>("hitpoints")
		.def("size", &hitpoints_wrap::size)
		.def("element", &hitpoints_wrap::element)
		.def("save", &hitpoints_wrap::save)
		.def("load", &hitpoints_wrap::load);

	class_<Scene>("Scene")
		// .def_readonly("cmpTargets", &Scene::);
		.def("add", &Scene::add)
		.def("newMaterial", &Scene::newMaterial);


	// this - data
	def("getImage", getBlenderImage);

	// this - main functions
	def("createScene", createScene);
	def("renderReference", renderReference_wrap);
	def("renderNonTarget", renderNonTarget_wrap);
	def("collectHitpoints", collectHitpoints_one_wrap);
	def("collectHitpoints_all", collectHitpoints_all_wrap);
	def("progressivePhotonMapping", progressivePhotonMapping_target);

	// file.hpp
	def("writeAllPass", writeAllPass);
	def("writeLayer", writeLayer);
	def("loadLayer", loadLayer);
}