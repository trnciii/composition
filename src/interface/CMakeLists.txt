set(PYBIND11_PYTHON_VERSION 3.9)
add_subdirectory(pybind11)
include_directories(pybind11)

include_directories(${PROJECT_SOURCE_DIR}/src)

pybind11_add_module(composition
	../composition.cpp
	interface.cpp
	data.cpp
	renderer.cpp
	scene.cpp
)