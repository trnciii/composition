#define BOOST_PYTHON_STATIC_LIB
#define BOOST_DISABLE_PRAGMA_MESSAGE
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <algorithm>
#include <string>
#include <omp.h>

#include "cmp.hpp"
#include "Image.hpp"
#include "file.hpp"
#include "data.hpp"
#include "toString.hpp"

inline std::vector<float> getBlenderImage(const Image& im){
	std::vector<float> f(im.len()*4);
	for(int y=0; y<im.h; y++){
		for(int x=0; x<im.w; x++){
			int i_src = y*im.w + x;
			int i_dst = (im.h -y-1)*im.w + x;
			
			f[4*i_dst  ] = im.pixels[i_src].x;
			f[4*i_dst+1] = im.pixels[i_src].y;
			f[4*i_dst+2] = im.pixels[i_src].z;
			f[4*i_dst+3] = 1.0f;
		}
	}
	return f;
}

void pt(
	Image& result, const int spp, const Scene& scene)
{
	RNG* rngForEveryPixel = new RNG[result.len()];
	for(int i=0; i<result.len(); i++)
		rngForEveryPixel[i] = RNG(i);

	pathTracing(result.data(), result.w, result.h, spp, scene, rngForEveryPixel);

	delete[] rngForEveryPixel;
}

void ppm(Image& result,
	int nRay, int nPhoton, int iteration, float alpha, float R0,
	const Scene& scene)
{

	int nThreads = omp_get_max_threads();
	std::vector<RNG> rngs(0);
	for(int i=0; i<nThreads+1; i++)
		rngs.push_back(RNG(i));

	std::vector<hitpoint> hits;
	hits.reserve(result.len()*nRay);

	std::cout <<"collecting hitpoints" <<std::endl;

	collectHitpoints(hits, result.w, result.h, nRay, scene, rngs[nThreads]);
	for(hitpoint& hit : hits)hit.clear(R0);
	
	std::cout <<"radiance estimation\n"
	<<"|--------- --------- --------- --------- |\n"
	<<"|" <<std::flush;

	for(int i=0; i<iteration; i++){
		if((i*40)%iteration < 39) std::cout <<"+" <<std::flush;

		Tree photonmap = createPhotonmap(scene, nPhoton, rngs.data(), nThreads);
		accumulateRadiance(hits, photonmap, scene, alpha);
	}

	std::cout <<"|" <<std::endl;

	for(hitpoint& hit : hits)
		result.pixels[hit.pixel] += hit.tau * hit.weight / (float)iteration;
}

void pt_notTarget(Image& result, const int spp, const Scene& scene)
{
	RNG* rngForEveryPixel = new RNG[result.len()];
	for(int i=0; i<result.len(); i++)
		rngForEveryPixel[i] = RNG(i);

	pathTracing_notTarget(result.data(), result.w, result.h, spp, scene, rngForEveryPixel);

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
	int nThreads = omp_get_max_threads();
	std::vector<RNG> rngs(0);
	for(int i=0; i<nThreads; i++)
		rngs.push_back(RNG(i));

	for(hitpoint& hit : hits.data)hit.clear(R0);

	std::cout <<"|--------- --------- --------- --------- |\n" <<"|" <<std::flush;

	for(int i=0; i<iteration; i++){
		if((i*40)%iteration < 39) std::cout <<"+" <<std::flush;
		
		Tree photonmap = createPhotonmap_target(scene, nPhoton, target, rngs.data(), nThreads);
		accumulateRadiance(hits.data, photonmap, scene, alpha);
	}
	std::cout <<"|" <<std::endl;
}

void hitsToImage(const hitpoints_wrap& hits, Image& result, const boost::python::object& remap){
	std::cout <<"|--------- --------- --------- --------- |\n" <<"|" <<std::flush;

	for(int i=0; i<hits.data.size(); i++){
		const hitpoint& hit = hits.data[i];
		if(i%(hits.data.size()/39) == 0) std::cout <<"+" <<std::flush;

		const glm::vec3 t = boost::python::extract<glm::vec3>(remap(hit));
		result.pixels[hit.pixel] += hit.weight * t;
	}

	std::cout <<"|" <<std::endl;
}


void addMesh(Scene& scene, const boost::python::list& vertices, const boost::python::list& indices){
	using namespace boost::python;

	Mesh m;

	for(int i=0; i<len(vertices); i++){
		m.vertices.push_back({
			extract<glm::vec3>(vertices[i][0]),	// position
			extract<glm::vec3>(vertices[i][1])	// normal
		});
	}

	for(int i=0; i<len(indices); i++){
		const object& index = indices[i];
		m.indices.push_back({
			(extract<uint32_t>(index[0])),	// v0
			(extract<uint32_t>(index[1])),	// v1
			(extract<uint32_t>(index[2])),	// v2
			(extract<glm::vec3>(index[3])),	// normal
			(extract<bool>(index[4])),		// smooth	
			(extract<uint32_t>(index[5]))	// matgrial
		});
	}

	m.buildTree();
	scene.addMesh(m);
}

void setMaterial(Scene& scene, uint32_t id, Material m){
	scene.materials[id] = m;
}

void setCamera(Camera& camera, const boost::python::list& m, float focal){
	using namespace boost::python;

	camera.flen = focal;

	if(len(m)!=16){
		std::cout <<"wrong input" <<std::endl;
		return;
	}

	camera.position = glm::vec3(extract<float>(m[3]), extract<float>(m[7]), extract<float>(m[11]));

	camera.toWorld[0] = glm::vec3(extract<float>(m[ 0]), extract<float>(m[ 4]), extract<float>(m[ 8]));
	camera.toWorld[1] = glm::vec3(extract<float>(m[ 1]), extract<float>(m[ 5]), extract<float>(m[ 9]));
	camera.toWorld[2] = glm::vec3(extract<float>(m[ 2]), extract<float>(m[ 6]), extract<float>(m[10]));
}

std::string Scene_str(const Scene& s){return str(s);}

BOOST_PYTHON_MODULE(composition){
	using namespace boost::python;

	// basic data types
	class_<glm::vec3>("vec3", init<float, float, float>())
		.def_readwrite("x", &glm::vec3::x)
		.def_readwrite("y", &glm::vec3::y)
		.def_readwrite("z", &glm::vec3::z);

	class_<std::vector<glm::vec3>>("vec_vec3")
		.def(vector_indexing_suite<std::vector<glm::vec3>>());

	class_<std::vector<uint32_t>>("vec_uint32t")
		.def(vector_indexing_suite<std::vector<uint32_t>>());

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
		.def_readwrite("targetIDs", &Scene::targetMaterials)
		.def("addMaterial", &Scene::addMaterial)
		.def("setMaterial", setMaterial)
		.def("addSphere", &Scene::addSphere)
		.def("addMesh", addMesh)
		.def("createBoxScene", createScene)
		.def("__str__", Scene_str);
	
	class_<Camera>("Camera")
		.def_readwrite("toWorld", &Camera::toWorld)
		.def_readwrite("position", &Camera::position)
		.def_readwrite("focalLength", &Camera::flen)
		.def("setDirection", &Camera::setDirection)
		.def("setSpace", setCamera);

	enum_<Material::Type>("MtlType")
		.value("emit", Material::Type::EMIT)
		.value("lambert", Material::Type::LAMBERT)
		.value("glossy", Material::Type::GGX_REFLECTION)
		.value("glass", Material::Type::GLASS)
		.export_values();

	class_<Material>("Material")
		.def_readwrite("name", &Material::name)
		.def_readwrite("type", &Material::type)
		.def_readwrite("color", &Material::color)
		.def_readwrite("a", &Material::a)
		.def_readwrite("ior", &Material::ior);


	// images
	class_<Image>("Image", init<int, int>())
		.def_readwrite("w", &Image::w)
		.def_readwrite("h", &Image::h)
		.def_readwrite("pixels", &Image::pixels);


	// functions
	def("getImage", getBlenderImage);

	def("readPixels", readPixels);
	def("writePixels", writePixels);

	// renderers
	def("pt", pt);
	def("ppm", ppm);
	def("pt_notTarget", pt_notTarget);

	def("collectHits_target_exclusive", collectHits_target_exclusive_wrap);
	def("collectHits_target", collectHits_target_wrap);
	def("radiance_target", progressiveRadianceEstimate_target);
	
	def("hitsToImage_cpp", hitsToImage);
}