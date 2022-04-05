#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "data.hpp"


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


void init_data(py::module_& m);
void init_scene(py::module_& m);
void init_renderer(py::module_& m);
