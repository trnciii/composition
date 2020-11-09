#define IMPLEMENT_SPHERE
#define IMPLEMENT_SCENE
#include "cmp.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <algorithm>

#include "Random.hpp"
#include "file.hpp"
#include "render_sub.hpp"

glm::vec3 pathTracingKernel_total(Ray ray, const Scene& scene, RNG* const rand){
	glm::vec3 throuput(1);
	float pTerminate = 1;

	while(rand->uniform() < pTerminate){
		Intersection is = intersect(ray, scene);
		const Material& mtl = scene.materials[is.mtlID];

		if(mtl.type == Material::Type::EMIT)
			return throuput*mtl.color;

		if(mtl.type == Material::Type::LAMBERT){
			throuput *= mtl.color/pTerminate;

			glm::vec3 tan[2];
			tangentspace(is.n, tan);
			glm::vec3 hemi = sampleCosinedHemisphere(rand->uniform(), rand->uniform());

			ray.o = offset(is.p, is.n);
			ray.d = (is.n*hemi.z) + (tan[0]*hemi.x) + (tan[1]*hemi.y);
		}

		pTerminate = std::max(throuput.x, std::max(throuput.y, throuput.z));
	}
	return glm::vec3(0);
}

void renderReference(const int w, const int h, const int spp, const Scene& scene){
	glm::vec3* reference = new glm::vec3[w*h];

	std::cout <<"path tracing for reference..." <<std::endl;
	#pragma omp parallel for schedule(dynamic)
	for(int i=0; i<w*h; i++){
		int xi = i%w;
		int yi = i/w;
		RNG rand(i);

		if(i%(w*h/20)==0){
			int n = i/((w*h)/20);
			printf("\r");
			printf("[ ");
			for(int m=0; m<n; m++)printf("+");
			for(int m=n; m<20; m++)printf("-");
			std::cout <<" ]" <<std::flush;
		}

		for(int n=0; n<spp; n++){
			double x = (double) (2*(xi+rand.uniform())-w )/h;
			double y = (double)-(2*(yi+rand.uniform())-h)/h;

			Ray view = scene.camera.ray(x, y);			
			reference[i] += pathTracingKernel_total(view, scene, &rand);
		}
		reference[i] /= spp;
	}

	std::string outDir("result");

	if(writeImage(reference, w, h, (outDir + "/reference.png").data()) == 1)
		std::cout <<" reference saved" <<std::endl;
	else std::cout <<"failed to save image" <<std::endl;
}

int createScene(Scene* s){
	s->camera.pos = glm::vec3(0,-10,4);
	s->camera.setDir(glm::vec3(0,1,0), glm::vec3(0,0,1));
	s->camera.flen = 2;

	s->materials[s->environment].type = Material::Type::EMIT;
	s->materials[s->environment].color = glm::vec3(0.05);


	uint32_t light = s->newMaterial(Material::Type::EMIT);
	s->materials[light].color = glm::vec3(30);

	uint32_t white = s->newMaterial(Material::Type::LAMBERT);
	s->materials[white].color = glm::vec3(0.6);

	uint32_t red = s->newMaterial(Material::Type::LAMBERT);
	s->materials[red].color = glm::vec3(0.85, 0.1, 0.1);

	uint32_t green = s->newMaterial(Material::Type::LAMBERT);
	s->materials[green].color = glm::vec3(0.1, 0.85, 0.1);

	uint32_t target = s->newMaterial(Material::Type::LAMBERT);
	s->materials[target].color = glm::vec3(0.6);
	s->aggregationTarget.push_back(target);

	// box
	s->add(Sphere(glm::vec3(-1e5, 0, 0), 1e5-4, green)); // left
	s->add(Sphere(glm::vec3( 1e5, 0, 0), 1e5-4, red)); // right
	s->add(Sphere(glm::vec3(0, 0, -1e5), 1e5, white)); // bottom
	s->add(Sphere(glm::vec3(0, 0,  1e5), 1e5-8, white)); // top
	s->add(Sphere(glm::vec3(0,  1e5, 0), 1e5-4, white)); // back
	s->add(Sphere(glm::vec3(1.5, 0.5, 1.5), 1.5, target));
	s->add(Sphere(glm::vec3(0,0,6), 0.5, light)); // light

	return 0;
}