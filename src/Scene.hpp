#pragma once

#include <vector>

#include "Camera.hpp"
#include "Sphere.hpp"
#include "data.hpp"

struct Scene{
	Camera camera;
	
	const uint32_t environment = 0;
	std::vector<Material> materials;
	std::vector<uint32_t> aggregationTarget;
	
	std::vector<Sphere> spheres;
	std::vector<uint32_t> lights;

	inline Scene():materials(1, Material(Material::Type::EMIT)){}
	
	void add(Sphere s);
	uint32_t newMaterial(Material::Type t);

};


#ifdef IMPLEMENT_SCENE

void Scene::add(Sphere s){
	if(materials.size() <= s.mtlID) return;

	if(materials[s.mtlID].type == Material::Type::EMIT)
		lights.push_back(spheres.size());

	spheres.push_back(s);
}

uint32_t Scene::newMaterial(Material::Type t){
	materials.push_back(Material(t));
	return materials.size()-1;
}

#endif