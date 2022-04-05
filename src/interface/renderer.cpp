#include "interface.hpp"

#include <iostream>

#include "composition.hpp"


void init_renderer(py::module_& m){
	py::class_<RNG>(m, "rng");

	m.def("createRNGs", createRNGVector);


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
			result[hit.pixel] += hit.weight * glm::vec3(t.at(0), t.at(1), t.at(2));
		}

		std::cout <<"|" <<std::endl;
	});

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