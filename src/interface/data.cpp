#include "interface.hpp"

#include "Image.hpp"
#include "file.hpp"
#include "data.hpp"
#include "Mesh.hpp"
#include "toString.hpp"


void init_data(py::module_& m){

	PYBIND11_NUMPY_DTYPE(glm::vec3, x, y, z);

	PYBIND11_NUMPY_DTYPE(Vertex, position, normal);
	PYBIND11_NUMPY_DTYPE(Index, v0, v1, v2, normal, use_smooth, mtlID);

	PYBIND11_NUMPY_DTYPE(hitpoint,
		p,
		n,
		ng,
		wo,
		mtlID,
		pixel,
		R,
		N,
		tau,
		weight,
		iteration,
		depth
	);


	py::bind_vector<std::vector<hitpoint>>(m, "Hitpoints", py::buffer_protocol())
		.def("write", writeVector<hitpoint>)
		.def("load", readVector<hitpoint>)
		.def("reset", [](std::vector<hitpoint>& self, float R0){
			using namespace std::placeholders;
			std::for_each(std::execution::unseq, self.begin(), self.end(), std::bind(&hitpoint::clear, _1, R0));
		})
		.def_buffer([](std::vector<hitpoint>& self){
			return py::buffer_info(
				self.data(),
				sizeof(hitpoint),
				py::format_descriptor<hitpoint>::format(),
				1,
				{self.size()},
				{sizeof(hitpoint)}
			);
		});

	m.def("read_hitpoints", [](const std::string& name){
		std::vector<hitpoint> ret;
		readVector(ret, name);
		return ret;
	});

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


	// images
	py::class_<Image>(m, "Image")
		.def(py::init<int, int>())
		.def(py::init<std::string>())
		.def_property_readonly("size", [](const Image& self){return py::make_tuple(self.width(), self.height());})
		.def_property("pixels",
			[](Image& self){
				return (py::array_t<float>)py::buffer_info(
					(float*)self.data(),
					sizeof(float),
					py::format_descriptor<float>::format(),
					3,
					{self.height(), self.width(), 3},
					{3*self.width()*sizeof(float), 3*sizeof(float), sizeof(float)}
				);
			},
			[](Image& self, py::array_t<float>& pixels){
				if(pixels.ndim() == 3){
					self.resize(pixels.shape(1), pixels.shape(0));
					self.setPixels(pixels.data(), pixels.size());
				}
				else if(pixels.ndim() == 1){
					self.setPixels(pixels.data(), pixels.size());
				}
				else assert(false);
			})
		.def("write", &Image::save)
		.def("load", &Image::load)
		.def("__len__", [](const Image& self){return self.size();});
}