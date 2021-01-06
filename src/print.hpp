#pragma once

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <glm/glm.hpp>
#include "data.hpp"
#include "Scene.hpp"

void printRule(){std::cout <<"----------  ----------  ----------"<<std::endl;}

void printBr(){putchar('\n');}

void print(const char* s1, const char* s2 = "\n"){
	std::cout <<s1 <<s2;
}

void print(const float& v, const char* s = "\n"){
	float exp = log10(fabs(v));
	if(-3<exp && exp<3) printf("%10.5f%s", v, s);
	else printf("%10.1e%s", v, s);
}

void print(const int& v, const char* s = "\n"){
	float exp = log10(fabs(v));
	if(exp<4) printf("%4d%s", v, s);
	else printf("%4e%s", (float)v, s);
}

void print(const uint32_t& v, const char* s = "\n"){
	float exp = log10(fabs(v));
	if(exp<(4)) printf("%4d%s", v, s);
	else printf("%4e%s", (float)v, s);
}

void print(const glm::vec3& a, const char* s = "\n"){
	print(a.x, " ");
	print(a.y, " ");
	print(a.z, s);
}

void print(const Ray& a, const char* s = "\n"){
	print(a.o, "  | ");
	print(a.d, s);
}

void print(const Sphere& sph, const char* str = "\n"){
	print(sph.p, " | ");
	print(sph.r, " | ");
	printf("mtl: %2d%s", sph.mtlID, str);
}

void print(const Material& m, const char* str = "\n"){
	std::cout <<std::setw(7);
	if(m.type == Material::Type::EMIT) std::cout <<"EMIT";
	if(m.type == Material::Type::LAMBERT) std::cout <<"LAMBERT";
	if(m.type == Material::Type::GGX_REFLECTION) std::cout <<"GGX";
	if(m.type == Material::Type::GLASS) std::cout <<"GLASS";
	
	std::cout <<" | ";
	print(m.color, " | ");
	print(m.a, " | ");
	print(m.ior, " | ");

	std::cout <<str;
}

void print(const Camera& a, const char* str="\n"){
	print(a.pos, " | ");
	print(a.basis[2], " | ");
	print(a.flen, str);
}

void print(const Mesh& m, const char* str="\n"){
	printf("positions\n");
	for(const Vertex& v : m.vertices){
		print(v.position);
	}

	printf("normals\n");
	for(const Vertex& v : m.vertices)print(v.normal);

	printf("faces\n");
	for(const Index& index : m.indices){
		print(index.v0, " | ");
		print(index.v1, " | ");
		print(index.v2, " | ");
		printf("mtl: %2d%s", index.mtlID, str);
	}
}

void print(const Scene& s){
	puts("[ scene ]");
	printRule();

	puts("camera");
	print(s.camera);
	printBr();

	puts("materials");
	for(int i=0; i<s.materials.size(); i++){
		printf("[%2d] ", i);
		if(std::find(s.cmpTargets.begin(), s.cmpTargets.end(), i) != s.cmpTargets.end())
			print(s.materials[i], "composite\n");
		else
			print(s.materials[i]);
	}
	printBr();

	puts("spheres");
	for(int i=0; i<s.spheres.size(); i++){
		printf("[%2d]", i);
		print(s.spheres[i]);
	}
	printBr();

	for(int i=0; i<s.meshes.size(); i++){
		printf("mesh[%2d]\n", i);
		print(s.meshes[i]);
	}
	printBr();
	
	printRule();
}