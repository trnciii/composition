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

unsigned char tonemap(double c){
	int c_out = 255*pow(c,(1/2.2)) +0.5;
	if(255 < c_out)c_out = 255;
	if(c_out < 0)c_out = 0;
	return c_out&0xff;
}

int writeImage(glm::vec3* color, int w, int h, const char* name){
	unsigned char *tone = new unsigned char[3*w*h];
	for(int i=0; i<w*h; i++){
		tone[3*i  ] = tonemap(color[i].x);
		tone[3*i+1] = tonemap(color[i].y);
		tone[3*i+2] = tonemap(color[i].z);
	}

	int result = stbi_write_png(name, w, h, 3, tone, 3*w);

	delete[] tone;
	return result;
}

glm::vec3 offset(glm::vec3 pos, glm::vec3 dir){return pos + (dir*1e-6f);}

void tangentspace(glm::vec3 n, glm::vec3 basis[2]){
	int sg =(n.z < 0) ?-1 :1;
	double a = -1.0/(sg+n.z);
	double b = n.x * n.y * a;
	basis[0] = glm::vec3(
		1.0 + sg * n.x*n.x * a,
		sg*b,
		-sg*n.x
	);
	basis[1] = glm::vec3(
		b,
		sg + n.y*n.y * a,
		-n.y
	);
}

glm::vec3 sampleCosinedHemisphere(double u1, double u2, double* p = nullptr){
	u2 *= 2*kPI;
	double r = sqrt(u1);
	double z = sqrt(1-u1);

	if(p)*p = z/kPI;
	return glm::vec3(r*cos(u2), r*sin(u2), z);
}

Intersection intersect(const Ray& ray, const Scene& scene){
	Intersection is;
		is.dist = kHUGE;
		is.mtlID = scene.environment;

	for(const Sphere& s : scene.spheres)
		s.intersect(&is, ray);

	if(glm::dot(is.n, ray.d)>0){
		is.n *= -1;
	}

	return is;
}

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

	std::string outDir("result_devCA");

	if(writeImage(reference, w, h, (outDir + "/reference.png").data()) == 1)
		std::cout <<" reference saved" <<std::endl;
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