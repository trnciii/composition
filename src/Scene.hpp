#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Camera.hpp"
#include "Sphere.hpp"
#include "data.hpp"
#include "Mesh.hpp"

struct Scene{
	Camera camera;
	
	const uint32_t environment = 0;
	std::vector<Material> materials;
	std::vector<uint32_t> cmpTargets;
	
	std::vector<Sphere> spheres;
	std::vector<uint32_t> lights;

	std::vector<Mesh> meshes;

	inline Scene():materials(1, Material(Material::Type::EMIT)){}
	
	inline void add(Sphere s){
		if(materials.size() <= s.mtlID) return;

		if(materials[s.mtlID].type == Material::Type::EMIT)
			lights.push_back(spheres.size());

		spheres.push_back(s);
	}

	inline void add(Mesh m){meshes.push_back(m);}

	inline uint32_t newMaterial(Material::Type t){
		materials.push_back(Material(t));
		return materials.size()-1;
	}

};