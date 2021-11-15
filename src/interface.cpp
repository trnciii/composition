#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

#include <algorithm>
#include <string>
#include <omp.h>
#include <iostream>

#include "composition.hpp"
#include "Image.hpp"
#include "file.hpp"
#include "data.hpp"
#include "toString.hpp"


namespace py = pybind11;


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
	const py::function& remap)
{
	std::cout <<"|--------- --------- --------- --------- |\n" <<"|" <<std::flush;

	for(int i=0; i<hits.size(); i++){
		const hitpoint& hit = hits[i];
		if(i%(hits.size()/39) == 0) std::cout <<"+" <<std::flush;

		const glm::vec3 t = remap(hit).cast<glm::vec3>();
		result.pixels[hit.pixel] += hit.weight * t;
	}

	std::cout <<"|" <<std::endl;
}


void addMesh(Scene& scene,
	const std::vector<py::list>& vertices,
	const std::vector<py::list>& indices,
	std::string name)
{
	Mesh m;
	m.name = name;

	for(int i=0; i<vertices.size(); i++){
		m.vertices.push_back({
			vertices[i][0].cast<glm::vec3>(),	// position
			vertices[i][1].cast<glm::vec3>()	// normal
		});
	}

	for(int i=0; i<indices.size(); i++){
		const py::list& index = indices[i];
		m.indices.push_back({
			index[0].cast<uint32_t>(),	// v0
			index[1].cast<uint32_t>(),	// v1
			index[2].cast<uint32_t>(),	// v2
			index[3].cast<glm::vec3>(),	// normal
			index[4].cast<bool>(),		// smooth
			index[5].cast<uint32_t>()	// material
		});
	}

	m.buildTree();
	scene.addMesh(m);
}

void setCamera(Camera& camera, const std::vector<float>& m, float focal){
	camera.flen = focal;

	if(m.size()!=16){
		std::cout <<"wrong input" <<std::endl;
		return;
	}

	camera.position = glm::vec3(m[3], m[7], m[11]);

	camera.toWorld[0] = glm::vec3(m[ 0], m[ 4], m[ 8]);
	camera.toWorld[1] = glm::vec3(m[ 1], m[ 5], m[ 9]);
	camera.toWorld[2] = glm::vec3(m[ 2], m[ 6], m[10]);
}


PYBIND11_MODULE(composition, m){

	// basic data types
	py::class_<glm::vec3>(m, "vec3")
		.def(py::init<float, float, float>())
		.def_readwrite("x", &glm::vec3::x)
		.def_readwrite("y", &glm::vec3::y)
		.def_readwrite("z", &glm::vec3::z)

		.def(py::self + glm::vec3())
		.def(py::self * glm::vec3())
		.def(py::self - glm::vec3())
		.def(py::self / glm::vec3())

		.def(py::self + float())
		.def(py::self - float())
		.def(py::self * float())
		.def(py::self / float())
		.def(float() + py::self)
		.def(float() - py::self)
		.def(float() * py::self)

		.def(py::self += glm::vec3())
		.def(py::self -= glm::vec3())
		.def(py::self *= glm::vec3())
		.def(py::self /= glm::vec3());


	// class_<std::vector<glm::vec3>>("vec_vec3")
		// .def(vector_indexing_suite<std::vector<glm::vec3>>());

	// class_<std::vector<uint32_t>>("vec_uint32t")
		// .def(vector_indexing_suite<std::vector<uint32_t>>());

	// class_<std::vector<float>>("vec_float")
		// .def(vector_indexing_suite<std::vector<float>>());


	py::class_<hitpoint>(m, "hitpoint")
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

	// py::class_<std::vector<hitpoint>>("vec_hitpoint")
	// 	.def(vector_indexing_suite<std::vector<hitpoint>>())
	// 	.def("save", save_hitpoints)
	// 	.def("load", load_hitpoints)
	// 	.def("clear", clearHitpoints);

	py::class_<std::vector<RNG>>(m, "rngs");

	m.def("createRNGs", createRNGVector);

	// scene
	py::class_<Scene>(m, "Scene")
		.def_readwrite("camera", &Scene::camera)
		.def_readwrite("materials", &Scene::materials)
		.def_readwrite("targetIDs", &Scene::targetMaterials)
		.def("addMaterial", &Scene::addMaterial)
		.def("setMaterial", setMaterial)
		.def("addSphere", &Scene::addSphere)
		.def("addMesh", addMesh)
		.def("createBoxScene", createScene)
		.def("__str__", Scene_str);
	
	py::class_<Camera>(m, "Camera")
		.def_readwrite("toWorld", &Camera::toWorld)
		.def_readwrite("position", &Camera::position)
		.def_readwrite("focalLength", &Camera::flen)
		.def("setDirection", &Camera::setDirection)
		.def("setSpace", setCamera);

	py::enum_<Material::Type>(m, "MtlType")
		.value("emit", Material::Type::EMIT)
		.value("lambert", Material::Type::LAMBERT)
		.value("glossy", Material::Type::GGX_REFLECTION)
		.value("glass", Material::Type::GLASS)
		.export_values();

	py::class_<Material>(m, "Material")
		.def_readwrite("name", &Material::name)
		.def_readwrite("type", &Material::type)
		.def_readwrite("color", &Material::color)
		.def_readwrite("a", &Material::a)
		.def_readwrite("ior", &Material::ior);


	// images
	py::class_<Image>(m, "Image")
		.def(py::init<int, int>())
		.def(py::init<std::string>())
		.def_readwrite("w", &Image::w)
		.def_readwrite("h", &Image::h)
		.def_readwrite("pixels", &Image::pixels)
		.def("save", &Image::save)
		.def("load", load_image)
		.def("toList", toBlenderImage);


	// renderers
	m.def("pt", PT);
	m.def("ppm", PPM);
	m.def("pt_notTarget", PT_notTarget);

	m.def("collectHits_target_exclusive", collectHitpoints_target_exclusive);
	m.def("collectHits_target", collectHitpoints_target);
	
	m.def("radiance_ppm", radiance_PPM);
	m.def("radiance_pt", radiance_PT);

	m.def("hitsToImage_cpp", hitsToImage);
}