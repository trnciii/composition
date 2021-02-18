#pragma once

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

	inline Scene():materials(1, Material()){}
	
	inline void addSphere(glm::vec3 c, float r, uint32_t m){
		if(materials.size() <= m) return;

		if(materials[m].type == Material::Type::EMIT)
			lights.push_back(spheres.size());

		spheres.push_back(Sphere(c, r, m));
	}

	inline void addMesh(Mesh m){
		for(const Index& i : m.indices)if(materials.size() <= i.mtlID)return;
		meshes.push_back(m);
	}

	inline uint32_t addMaterial(Material m){
		materials.push_back(m);
		return materials.size()-1;
	}
};