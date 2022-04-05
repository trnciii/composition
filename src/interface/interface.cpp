#include "interface.hpp"

PYBIND11_MODULE(composition, m){
	init_data(m);
	init_scene(m);
	init_renderer(m);
}
