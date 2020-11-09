#define BOOST_PYTHON_STATIC_LIB
#define BOOST_DISABLE_PRAGMA_MESSAGE
#include <boost/python.hpp>

#include "cmp.hpp"

BOOST_PYTHON_MODULE(composition) {
	using namespace boost::python;

	class_<Scene>("Scene");
		// .def("add", &Scene::add)
		// .def("newMaterial", &Scene.newMaterial);

	def("createScene", createScene);
	def("renderReference", renderReference);

}