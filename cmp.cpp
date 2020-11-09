#define IMPLEMENT_SPHERE
#define IMPLEMENT_SCENE
#include "cmp.hpp"

#include <stb/stb_image_write.h>

#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <algorithm>

#include "Random.hpp"
#include "render_sub.hpp"

void renderReference(glm::vec3* const result, const int w, const int h, const int spp, const Scene& scene, RNG* const rngs){
	#pragma omp parallel for schedule(dynamic)
	for(int i=0; i<w*h; i++){
		int xi = i%w;
		int yi = i/w;
		RNG& rand = rngs[i];

		for(int n=0; n<spp; n++){
			double x = (double) (2*(xi+rand.uniform())-w )/h;
			double y = (double)-(2*(yi+rand.uniform())-h)/h;

			Ray view = scene.camera.ray(x, y);			
			result[i] += pathTracingKernel_total(view, scene, &rand);
		}
		result[i] /= spp;
	}
}



void renderNonTarget(glm::vec3* const result, const int w, const int h, const int spp, const Scene& scene, RNG* const rngs){
	#pragma omp parallel for schedule(dynamic)
	for(int i=0; i<w*h; i++){
		int xi = i%w;
		int yi = i/w;
		RNG& rand = rngs[i];

		for(int n=0; n<spp; n++){
			double x = (double) (2*(xi+rand.uniform())-w)/h;
			double y = (double)-(2*(yi+rand.uniform())-h)/h;

			Ray view = scene.camera.ray(x, y);			
			result[i] += pathTracingKernel_nonTarget(view, scene, &rand);
		}
		result[i] /= spp;
	}
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