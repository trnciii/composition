#define BOOST_PYTHON_STATIC_LIB
#define BOOST_DISABLE_PRAGMA_MESSAGE
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

// #include <execution>
#include <algorithm>
#include <string>

#include "cmp.hpp"
#include "RenderPass.hpp"
#include "file.hpp"
#include "data.hpp"
#include "print.hpp"

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

void pt(
	RenderPass& pass, const int layer, const int spp, const Scene& scene)
{
	glm::vec3* const result = pass.data(layer);
	const int w = pass.width;
	const int h = pass.height;

	RNG* rngForEveryPixel = new RNG[w*h];
	for(int i=0; i<w*h; i++)
		rngForEveryPixel[i] = RNG(i);

	pathTracing(result, w, h, spp, scene, rngForEveryPixel);

	delete[] rngForEveryPixel;
}

void ppm(RenderPass& pass, const int layer,
	int nRay, int nPhoton, int iteration, float alpha, float R0,
	const Scene& scene)
{

	RNG rng(0);

	std::vector<hitpoint> hits;
	hits.reserve(pass.length*nRay);

	collectHitpoints(hits, pass.width, pass.height, nRay, scene, rng);
	for(hitpoint& hit : hits)hit.clear(R0);
	for(int i=0; i<iteration; i++){
		Tree photonmap = createPhotonmap(scene, nPhoton, rng);
		accumulateRadiance(hits, photonmap, scene, alpha);
	}

	glm::vec3* image = pass.data(layer);
	for(hitpoint& hit : hits)
		image[hit.pixel] += hit.tau * hit.weight / (float)iteration;
}

void pt_notTarget(
	RenderPass& pass, const int layer, const int spp, const Scene& scene)
{
	glm::vec3* const result = pass.data(layer);
	const int w = pass.width;
	const int h = pass.height;

	RNG* rngForEveryPixel = new RNG[w*h];
	for(int i=0; i<w*h; i++)
		rngForEveryPixel[i] = RNG(i);

	pathTracing_notTarget(result, w, h, spp, scene, rngForEveryPixel);

	delete[] rngForEveryPixel;
}

class hitpoints_wrap{
public:
	std::vector<hitpoint> data;

	hitpoints_wrap():data(0){}
	hitpoints_wrap(std::vector<hitpoint> h):data(h){}

	std::string save(const std::string& s){
		std::string result;
		if(writeVector(data, s))
			result = "Saved hitpoints as " + s + ": size = " + std::to_string(data.size());
		else
			result = "failed to save hitpoints: " + s;
		std::cout <<result <<std::endl;
		return result;
	}

	std::string load(const std::string& s){
		std::string result;
		if(readVector(data, s))
			result = "Read hitpoints:" + s + ": size = " + std::to_string(data.size());
		else
			result = "failed to read hitpoints: " + s;
		std::cout <<result <<std::endl;
		return result;
	}
};

hitpoints_wrap collectHits_target_wrap(const int depth, const int w, const int h, const int nRay,
	const Scene& scene, const uint32_t target)
// todo: passing rng state
{
	std::vector<hitpoint> hits;
	hits.reserve(w*h*nRay);
	RNG rng(0);

	collectHitpoints_target(hits, target, depth, w, h, nRay, scene, rng);

	return hitpoints_wrap(hits);
}

hitpoints_wrap collectHits_target_exclusive_wrap(const int depth, const int w, const int h, const int nRay,
	const Scene& scene, const uint32_t target)
// todo: passing rng state
{
	std::vector<hitpoint> hits;
	hits.reserve(w*h*nRay);
	RNG rng(0);

	collectHitpoints_target_exclusive(hits, target, depth, w, h, nRay, scene, rng);

	return hitpoints_wrap(hits);
}

void progressiveRadianceEstimate_target(hitpoints_wrap& hits,
	const float R0, const int iteration, const int nPhoton, const float alpha,
	const Scene& scene, const uint32_t target)
{
	RNG rng;
	for(hitpoint& hit : hits.data)hit.clear(R0);
	for(int i=0; i<iteration; i++){
		Tree photonmap = createPhotonmap_target(scene, nPhoton, target, rng);
		accumulateRadiance(hits.data, photonmap, scene, alpha);
	}
}

void hitsToImage(const hitpoints_wrap& hits, RenderPass& pass, const int layer,
	const boost::python::object& remap)
{
	std::cout <<"using cpp for replacement" <<std::endl;

	std::vector<glm::vec3> image(pass.length);
	std::vector<glm::vec3> replacement(hits.data.size());

	std::transform(/*std::execution::par_unseq,*/
		hits.data.begin(), hits.data.end(),
		replacement.begin(),
		[&remap](const hitpoint& hit){
			glm::vec3 t = boost::python::extract<glm::vec3>(remap(hit));
			return hit.weight*t;
		});

	for(int i=0; i<hits.data.size(); i++)
		image[hits.data[i].pixel] += replacement[i];

	pass.setLayer(layer, image.data());
}


void addMesh(Scene& scene, const boost::python::list& vertices, const boost::python::list& indices){
	using namespace  boost::python;

	Mesh m;

	for(int i=0; i<len(vertices); i++){
		const boost::python::object& v = vertices[i];
		m.vertices.push_back({
			{extract<float>(v[0]), extract<float>(v[1]), extract<float>(v[2])},
			{extract<float>(v[3]), extract<float>(v[4]), extract<float>(v[5])} });
	}

	for(int i=0; i<len(indices); i++){
		const boost::python::object& index = indices[i];
		m.indices.push_back({
			(extract<uint32_t>(index[0])),
			(extract<uint32_t>(index[1])),
			(extract<uint32_t>(index[2])),
			(extract<uint32_t>(index[3]))});
	}

	m.update();
	scene.meshes.push_back(m);
}

void setCamera(Camera& camera, const boost::python::list& m, float focal){
	using namespace boost::python;

	camera.flen = focal;

	if(len(m)!=16){
		std::cout <<"wrong input" <<std::endl;
		return;
	}

	camera.pos = glm::vec3(extract<float>(m[3]), extract<float>(m[7]), extract<float>(m[11]));

	camera.basis[0] = glm::vec3(extract<float>(m[0]), extract<float>(m[4]), extract<float>(m[8]));
	camera.basis[1] = glm::vec3(extract<float>(m[1]), extract<float>(m[5]), extract<float>(m[9]));
	camera.basis[2] = glm::vec3(extract<float>(m[2]), extract<float>(m[6]), extract<float>(m[10]));
}

void print_scene(const Scene& scene){
	print(scene);
}

BOOST_PYTHON_MODULE(composition){
	using namespace boost::python;

	// basic data types
	class_<glm::vec3>("vec3", init<float, float, float>())
		.def_readwrite("x", &glm::vec3::x)
		.def_readwrite("y", &glm::vec3::y)
		.def_readwrite("z", &glm::vec3::z);

	class_<std::vector<float>>("vec_float")
		.def(vector_indexing_suite<std::vector<float>>());


	class_<hitpoint>("hitpoint")
		.def_readwrite("p", &hitpoint::p)
		.def_readwrite("n", &hitpoint::n)
		.def_readwrite("wo", &hitpoint::wo)
		.def_readwrite("mtlID", &hitpoint::mtlID)
		.def_readwrite("pixel", &hitpoint::pixel)
		.def_readwrite("R", &hitpoint::R)
		.def_readwrite("N", &hitpoint::N)
		.def_readwrite("tau", &hitpoint::tau)
		.def_readwrite("weight", &hitpoint::weight)
		.def_readwrite("iteration", &hitpoint::iteration)
		.def_readwrite("depth", &hitpoint::depth);

	class_<std::vector<hitpoint>>("vec_hitpoint")
		.def(vector_indexing_suite<std::vector<hitpoint>>());

	class_<hitpoints_wrap>("hitpoints")
		.def_readonly("data", &hitpoints_wrap::data)
		.def("save", &hitpoints_wrap::save)
		.def("load", &hitpoints_wrap::load);


	// scene
	class_<Scene>("Scene")
		.def_readwrite("camera", &Scene::camera)
		.def_readwrite("materials", &Scene::materials)
		.def("addMaterial", &Scene::addMaterial);
		// .def_readonly("cmpTargets", &Scene::);
		// .def("newMaterial", &Scene::newMaterial);

	def("createScene", createScene);
	def("addMesh", addMesh);
	def("print_scene", print_scene);

	class_<Camera>("Camera")
		.def_readwrite("position", &Camera::pos)
		.def_readwrite("focalLength", &Camera::flen)
		.def("setSpace", &Camera::setDir);

	def("setCamera", setCamera);

	enum_<Material::Type>("MtlType")
		.value("emit", Material::Type::EMIT)
		.value("lambert", Material::Type::LAMBERT)
		.value("glossy", Material::Type::GGX_REFLECTION)
		.value("glass", Material::Type::GLASS)
		.export_values();

	class_<Material>("Material")
		.def_readwrite("type", &Material::type)
		.def_readwrite("color", &Material::color)
		.def_readwrite("a", &Material::a)
		.def_readwrite("ior", &Material::ior);


	// render pass
	class_<RenderPass>("RenderPass", init<int, int>())
		.def_readonly("width", &RenderPass::width)
		.def_readonly("height", &RenderPass::height)
		.def_readonly("length", &RenderPass::length)
		.def_readonly("nLayers", &RenderPass::nLayer)
		.def("addLayer", &RenderPass::addLayer)
		.def("clear", &RenderPass::clear)
		.def("set", &RenderPass::setPixel);

	def("getImage", getBlenderImage);
	
	def("writeAllPass", writeAllPass);
	def("writeLayer", writeLayer);
	def("loadLayer", loadLayer);

	// renderers
	def("pt", pt);
	def("ppm", ppm);
	def("pt_notTarget", pt_notTarget);

	def("collectHits_target_exclusive", collectHits_target_exclusive_wrap);
	def("collectHits_target", collectHits_target_wrap);
	def("radiance_target", progressiveRadianceEstimate_target);
	
	def("hitsToImage_cpp", hitsToImage);
}