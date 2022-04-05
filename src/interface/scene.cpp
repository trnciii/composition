#include "interface.hpp"

#include "toString.hpp"
#include "composition.hpp"

void init_scene(py::module_& m){
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
}