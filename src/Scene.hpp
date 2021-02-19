#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "Camera.hpp"
#include "Sphere.hpp"
#include "data.hpp"
#include "Mesh.hpp"

struct Scene{
	Camera camera;
	
	std::vector<Material> materials;
	const uint32_t environment = 0;
	std::vector<uint32_t> lights;
	std::vector<uint32_t> targetMaterials;
	
	std::vector<Sphere> spheres;
	std::vector<Mesh> meshes;

	inline Scene():materials(1, Material()){
		materials[0].name = "env";
		materials[0].color = glm::vec3(0);
		materials[0].type = Material::Type::EMIT;
	}
	
	inline void addSphere(glm::vec3 c, float r, uint32_t m, std::string n=""){
		if(materials.size() <= m) return;

		if(materials[m].type == Material::Type::EMIT)
			lights.push_back(spheres.size());

		if(n.size()==0) n = "Sphere." + std::to_string(spheres.size());
		spheres.push_back(Sphere(c, r, m, n));
	}

	inline void addMesh(Mesh m){
		for(const Index& i : m.indices)if(materials.size() <= i.mtlID)return;

		if(m.name.size()==0) m.name = "Mesh." + std::to_string(meshes.size());
		meshes.push_back(m);
	}

	inline uint32_t addMaterial(Material m){
		if(m.name.size()==0)m.name = "Mtl." + std::to_string(materials.size());
		materials.push_back(m);
		return materials.size()-1;
	}
};