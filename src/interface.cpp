#define BOOST_PYTHON_STATIC_LIB
#define BOOST_DISABLE_PRAGMA_MESSAGE
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <algorithm>
#include <string>
#include <omp.h>

#include "composition.hpp"
#include "Image.hpp"
#include "file.hpp"
#include "data.hpp"
#include "toString.hpp"


std::string Scene_str(const Scene& s){return str(s);}

void setMaterial(Scene& scene, uint32_t id, Material m){scene.materials[id] = m;}


std::string Hitpoint_str(const hitpoint& h){return str(h);}

std::string save_hitpoints(std::vector<hitpoint>& data, const std::string& name){
	std::string result;
	if(writeVector(data, name))
		return "Saved " + std::to_string(data.size()) + " hitpoints as " + name;
	else
		return "failed to save hitpoints as " + name;
}

std::string load_hitpoints(std::vector<hitpoint>& data, const std::string& name){
	std::string result;
	if(readVector(data, name))
		return "Read " + std::to_string(data.size()) +  " hitpoints from " + name;
	else
		return "failed to read hitpoints from " + name;
}

void clearHitpoints(std::vector<hitpoint>& hits, float R0){
	for(hitpoint& hit : hits)hit.clear(R0);
}


void load_image(Image& image, const std::string name){image.load(name);}

inline std::vector<float> toBlenderImage(const Image& im){
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

void hitsToImage(const std::vector<hitpoint>& hits, Image& result,
	const boost::python::object& remap)
{
	std::cout <<"|--------- --------- --------- --------- |\n" <<"|" <<std::flush;

	for(int i=0; i<hits.size(); i++){
		const hitpoint& hit = hits[i];
		if(i%(hits.size()/39) == 0) std::cout <<"+" <<std::flush;

		const glm::vec3 t = boost::python::extract<glm::vec3>(remap(hit));
		result.pixels[hit.pixel] += hit.weight * t;
	}

	std::cout <<"|" <<std::endl;
}


void addMesh(Scene& scene,
	const boost::python::list& vertices,
	const boost::python::list& indices,
	std::string name)
{
	using namespace boost::python;

	Mesh m;
	m.name = name;

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
		.def_readwrite("depth", &hitpoint::depth)
		.def("__str__", Hitpoint_str);

	class_<std::vector<hitpoint>>("vec_hitpoint")
		.def(vector_indexing_suite<std::vector<hitpoint>>())
		.def("save", save_hitpoints)
		.def("load", load_hitpoints)
		.def("clear", clearHitpoints);

	class_<std::vector<RNG>>("rngs");

	def("createRNGs", createRNGVector);

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
		.def_readwrite("pixels", &Image::pixels)
		.def(init<std::string>())
		.def("save", &Image::save)
		.def("load", load_image)
		.def("toList", toBlenderImage);


	// renderers
	def("pt", PT);
	def("ppm", PPM);
	def("pt_notTarget", PT_notTarget);

	def("collectHits_target_exclusive", collectHitpoints_target_exclusive);
	def("collectHits_target", collectHitpoints_target);
	
	def("radiance_ppm", radiance_PPM);
	def("radiance_pt", radiance_PT);

	def("hitsToImage_cpp", hitsToImage);
}