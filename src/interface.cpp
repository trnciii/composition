#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

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


#define PROPERTY_FLOAT3(Class, member)                  \
	[](const Class& self){                                \
		return (py::array_t<float>)py::buffer_info(         \
			(float*)&self.member,                             \
			sizeof(float),                                    \
			py::format_descriptor<float>::format(),           \
			1,{3},{sizeof(float)}                             \
			);                                                \
	},                                                    \
	[](Class& self, py::array_t<float>& x){               \
		auto r = x.mutable_unchecked<1>();                  \
		assert(r.shape(0) == 3);                            \
		memcpy(&self.member, x.data(0), sizeof(glm::vec3)); \
	}


PYBIND11_MAKE_OPAQUE(std::vector<hitpoint>);

PYBIND11_MODULE(composition, m){

	py::bind_vector<std::vector<hitpoint>>(m, "Hitpoints");

	PYBIND11_NUMPY_DTYPE(glm::vec3, x, y, z);

	PYBIND11_NUMPY_DTYPE(Vertex, position, normal);
	PYBIND11_NUMPY_DTYPE(Index, v0, v1, v2, normal, use_smooth, mtlID);

	// PYBIND11_NUMPY_DTYPE(hitpoint,
	// 	p,
	// 	n,
	// 	ng,
	// 	wo,
	// 	mtlID,
	// 	pixel,
	// 	R,
	// 	N,
	// 	tau,
	// 	weight,
	// 	iteration,
	// 	depth
	// );


	py::class_<hitpoint>(m, "hitpoint")
		.def_property("p", PROPERTY_FLOAT3(hitpoint, p))
		.def_property("n", PROPERTY_FLOAT3(hitpoint,n))
		.def_property("ng", PROPERTY_FLOAT3(hitpoint, ng))
		.def_property("wo", PROPERTY_FLOAT3(hitpoint,wo))
		.def_readwrite("mtlID", &hitpoint::mtlID)
		.def_readwrite("pixel", &hitpoint::pixel)
		.def_readwrite("R", &hitpoint::R)
		.def_readwrite("N", &hitpoint::N)
		.def_property("tau", PROPERTY_FLOAT3(hitpoint,tau))
		.def_property("weight", PROPERTY_FLOAT3(hitpoint,weight))
		.def_readwrite("iteration", &hitpoint::iteration)
		.def_readwrite("depth", &hitpoint::depth)
		.def("__str__", [](const hitpoint& h){return str(h);});

	m.def("save_hitpoints", [](std::vector<hitpoint>& data, const std::string& name){
		std::string result;
		if(writeVector(data, name))
			return "Saved " + std::to_string(data.size()) + " hitpoints as " + name;
		else
			return "failed to save hitpoints as " + name;
	});

	m.def("load_hitpoints", [](std::vector<hitpoint>& data, const std::string& name){
		std::string result;
		if(readVector(data, name))
			return "Read " + std::to_string(data.size()) +  " hitpoints from " + name;
		else
			return "failed to read hitpoints from " + name;
	});

	m.def("clear_hitpoints", [](std::vector<hitpoint>& hits, float R0){
		for(hitpoint& hit : hits)hit.clear(R0);
	});

	py::class_<RNG>(m, "rng");

	m.def("createRNGs", createRNGVector);

	// scene
	py::class_<Scene>(m, "Scene")
		.def(py::init<>())
		.def_readwrite("camera", &Scene::camera)
		.def_readwrite("materials", &Scene::materials)
		.def_readwrite("targetIDs", &Scene::targetMaterials)
		.def("addMaterial", &Scene::addMaterial)
		.def("setMaterial", [](Scene& scene, uint32_t id, Material m){scene.materials[id] = m;})
		.def("addSphere", [](Scene& self, py::array_t<float>& c, float r, uint32_t m, const std::string& n){
			glm::vec3 center(c.at(0), c.at(1), c.at(2));
			self.addSphere(center, r, m, n);
		})
		.def("addMesh", [](
			Scene& self,
			const py::array_t<Vertex>& vertices,
			const py::array_t<Index>& indices,
			std::string name)
		{
			Mesh m;
			m.name = name;

			m.vertices = std::vector<Vertex>(vertices.data(), vertices.data()+vertices.size());
			m.indices = std::vector<Index>(indices.data(), indices.data()+indices.size());

			m.buildTree();
			self.addMesh(m);
		})
		.def("createBoxScene", createScene)
		.def("__str__", [](const Scene& s){return str(s);});
	

	py::class_<Camera>(m, "Camera")
		.def_property("toWorld",
			[](const Camera& self){
				return (py::array_t<float>)py::buffer_info(
					(float*)&self.toWorld,
					sizeof(float),
					py::format_descriptor<float>::format(),
					2,
					{3, 3},
					{sizeof(glm::vec3), sizeof(float)}
					);
			},
			[](Camera& self, py::array_t<float>& x){
				auto r = x.unchecked<2>();
				assert(r.shape(0) == 3 && r.shape(1) == 3);
				for(int i=0; i<3; i++){
					for(int j=0; j<3; j++)
						self.toWorld[i][j] = r(j, i); // row major (numpy) to column major (glm)
				}
			})
		.def_property("position", PROPERTY_FLOAT3(Camera, position))
		.def_readwrite("focalLength", &Camera::flen)
		.def("setDirection", &Camera::setDirection);


	py::enum_<Material::Type>(m, "MtlType")
		.value("emit", Material::Type::EMIT)
		.value("lambert", Material::Type::LAMBERT)
		.value("glossy", Material::Type::GGX_REFLECTION)
		.value("glass", Material::Type::GLASS)
		.export_values();

	py::class_<Material>(m, "Material")
		.def(py::init<>())
		.def_readwrite("name", &Material::name)
		.def_readwrite("type", &Material::type)
		.def_property("color", PROPERTY_FLOAT3(Material, color))
		.def_readwrite("a", &Material::a)
		.def_readwrite("ior", &Material::ior);


	// images
	py::class_<Image>(m, "Image")
		.def(py::init<int, int>())
		.def(py::init<std::string>())
		.def_readwrite("w", &Image::w)
		.def_readwrite("h", &Image::h)
		.def_property("pixels",
			[](const Image& self){
				return (py::array_t<float>)py::buffer_info(
					(float*)self.pixels.data(),
					sizeof(float),
					py::format_descriptor<float>::format(),
					3,
					{self.h, self.w, 3},
					{3*self.w*sizeof(float), 3*sizeof(float), sizeof(float)}
				);
			},
			[](Image& self, py::array_t<float>& pixels){
				assert(pixels.ndim() == 3);
				self.h = pixels.shape(0);
				self.w = pixels.shape(1);
				size_t len = self.w * self.h;
				self.pixels.resize(len);
				memcpy(self.pixels.data(), pixels.data(), len*sizeof(glm::vec3));
			})
		.def("save", &Image::save)
		.def("load", [](Image& image, const std::string name){image.load(name);})
		.def("toList", [](const Image& im){
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
		})
		.def("__len__", [](const Image& self){return self.pixels.size();});


	// renderers
	m.def("pt", PT);
	m.def("ppm", PPM);
	m.def("pt_notTarget", PT_notTarget);

	m.def("collectHits_target_exclusive", collectHitpoints_target_exclusive);
	m.def("collectHits_target", collectHitpoints_target);
	
	m.def("radiance_ppm", radiance_PPM);
	m.def("radiance_pt", radiance_PT);

	m.def("hitsToImage_cpp", [](
		const std::vector<hitpoint>& hits,
		Image& result,
		const py::function& remap)
	{
		std::cout <<"|--------- --------- --------- --------- |\n" <<"|" <<std::flush;

		for(int i=0; i<hits.size(); i++){
			const hitpoint& hit = hits[i];
			if(i%(hits.size()/39) == 0) std::cout <<"+" <<std::flush;

			const py::array_t<float> t = remap(hit);
			result.pixels[hit.pixel] += hit.weight * glm::vec3(t.at(0), t.at(1), t.at(2));
		}

		std::cout <<"|" <<std::endl;
	});

	// nprr
	m.def("nprr", [](
		const int w, const int h, const Scene& scene,
		const int spp, std::vector<RNG>& rng_per_pixel,
		const std::vector< std::tuple<std::vector<float>, float, float> >& remaps_py)
	{
		std::vector<std::function<glm::vec3(float)>> remaps_cpp;
		for(const auto& remap : remaps_py){
			const auto& [image, min, max] = remap;
			remaps_cpp.push_back([image, min, max](float u){
				u = (u-min)/(max-min);

				int w = image.size()/3;
				int co = u*w;

				if(co < 0) co = 0;
				if(w-1 < co) co = w-1;

				return glm::vec3(image[3*co], image[3*co+1], image[3*co+2]);
			});
		}

		return nprr(w, h, scene, spp, rng_per_pixel, remaps_cpp);
	});
}
