#pragma once

#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <glm/glm.hpp>
#include "data.hpp"
#include "Scene.hpp"

std::string str(const int& v){
	float e = log10(fabs(v));
	char s[20];
	if(e<4) std::sprintf(s, "%4d", v);
	else std::sprintf(s, "%4e", float(v));
	return std::string(s);
}

std::string str(const uint32_t& v){
	float e = log10(fabs(v));
	char s[20];
	if(e<4) std::sprintf(s, "%4d", v);
	else std::sprintf(s, "%4e", float(v));
	return std::string(s);
}

std::string str(const float& v){
	float exp = log10(fabs(v));
	char s[20];
	if(-3<exp && exp<3)std::sprintf(s, "%9.5f", v);
	else std::sprintf(s, "%9.1e", v);
	return std::string(s);
}

std::string str(const glm::vec3& v){
	return str(v.x)+" "+str(v.y)+" "+str(v.z);
}

std::string str(const glm::vec4& v){
	return str(v.x)+" "+str(v.y)+" "+str(v.z)+" "+str(v.w);
}

std::string str(const Ray& ray){
	return str(ray.o) + " | " + str(ray.d);
}

std::string str(const Sphere& s){
	char m[20];
	std::sprintf(m, "material: %2d", s.mtlID);
	return str(s.p) + " | " + str(s.r) + " | " + m;
}

std::string str(const Material::Type& t){
	if(t == Material::Type::EMIT)			return "   EMIT";
	if(t == Material::Type::LAMBERT)		return "LAMBERT";
	if(t == Material::Type::GGX_REFLECTION) return "    GGX";
	if(t == Material::Type::GLASS)			return "  GLASS";
	return "----";
}

std::string str(const Material& m){
	std::string s = str(m.type);
	s += " | color:" + str(m.color);

	if(m.type == Material::Type::GGX_REFLECTION)
		s += " | roughness:" + str(m.a);
	if(m.type == Material::Type::GLASS)
		s += " | roughness:" + str(m.a) + " | ior:" + str(m.ior);

	return s;
}

std::string str(const Camera& c){
	glm::mat3 w = glm::transpose(c.toWorld);
	
	std::string s = "position:\n" + str(c.position) + "\n";
	s += "transform:\n" + str(w[0])+"\n" + str(w[1])+"\n" + str(w[2])+"\n";
	s += "focal:\n" + str(c.flen) + "\n";
	return s;
}

template<typename T> int getMaxNameSize(const std::vector<T>& v){
	if(v.size()==0) return 0;
	return std::max_element(v.begin(), v.end(),
		[](const T& a, const T& b){
			return a.name.size() < b.name.size();
		})->name.size();
}

std::string str(const Scene& scene){
	std::stringstream s;

	s <<"scene\n" <<"----------  ----------  ----------\n";

	s <<"camera\n" <<str(scene.camera);

	s <<"\nmaterials\n";{
		std::vector<Material> materials = scene.materials;
		std::sort(materials.begin(), materials.end(),
			[](const Material& a, const Material& b){
				std::string na = a.name;
				std::string nb = b.name;
				transform(na.begin(), na.end(), na.begin(), tolower);
				transform(nb.begin(), nb.end(), nb.begin(), tolower);
				return na < nb;
			});
		
		int width = getMaxNameSize(materials)+1;
		for(const Material& m : materials){
			s <<std::setw(width) <<std::left <<m.name <<": " <<str(m) <<"\n";
		}
	}


	s <<"\ntarget materials: [ ";
	for(uint32_t i : scene.targetMaterials){
		s <<scene.materials[i].name;
		if(i != scene.targetMaterials.back())
			s <<", ";
	}
	s <<" ]\n";


	s <<"\nspheres\n";{
		int width = getMaxNameSize(scene.spheres)+1;
		for(const Sphere& sphere : scene.spheres){
			std::string spstr = str(sphere);
			spstr.resize(spstr.size() - std::string("xx").size());
			s <<std::setw(width) <<sphere.name <<":"
				<<spstr <<scene.materials[sphere.mtlID].name <<"\n";
		}
	}


	s <<"\nmeshes\n";{
		int width = getMaxNameSize(scene.meshes)+1;
		for(int i=0; i<scene.meshes.size(); i++){
			s <<std::setw(width) <<std::left <<scene.meshes[i].name <<": ";

			// create the list of materials assigned to this mesh
			std::vector<uint32_t> mtls;
			for(const Index& i : scene.meshes[i].indices){
				if(std::find(mtls.begin(), mtls.end(), i.mtlID) == mtls.end())
					mtls.push_back(i.mtlID);
			}

			s <<scene.meshes[i].vertices.size() <<" vertices, "
				<<scene.meshes[i].indices.size() <<" triangles, "
				<<"materials: [ ";

			for(int i=0; i<mtls.size(); i++){
				s <<scene.materials[mtls[i]].name;
				if(i!=mtls.size()-1) s <<", ";
			}

			s <<" ]\n";
		}
	}

	s <<"----------  ----------  ----------\n";
	return s.str();
}

std::string str(const Mesh& mesh){
	std::stringstream s;
	s <<mesh.name <<"\n----------  ----------  ----------\n";

	s <<"material ids : [ ";
	std::vector<uint32_t> mtls;
	for(const Index& i : mesh.indices){
		if(std::find(mtls.begin(), mtls.end(), i.mtlID) == mtls.end())
			mtls.push_back(i.mtlID);
	}

	for(int i=0; i<mtls.size(); i++){
		s <<mtls[i];
		if(i!=mtls.size()-1) s <<", ";
	}
	s <<" ]\n";


	s <<"\nvertex [position, normal]\n";
	int wv = ceil(log10(mesh.vertices.size()));
	for(int i=0; i<mesh.vertices.size(); i++){
		const Vertex& v = mesh.vertices[i];
		s <<"[" <<std::setw(wv) <<i <<"] ";
		s <<str(v.position) <<" | " <<str(v.normal) <<"\n";
	}

	s <<"\nindex [triangle, normal, shading, material]\n";
	int wi = ceil(log10(mesh.indices.size()));
	for(int i=0; i<mesh.indices.size(); i++){
		const Index& index = mesh.indices[i];
		s <<"[" <<std::setw(wi) <<i <<"]  ";
		
		s <<std::setw(wv) <<index.v0 <<" - ";
		s <<std::setw(wv) <<index.v1 <<" - ";
		s <<std::setw(wv) <<index.v2 <<" | ";

		s <<str(index.normal) <<" | ";

		if(index.use_smooth)
			s <<"smooth | ";
		else
			s <<"  flat | ";

		s <<index.mtlID;

		s <<"\n";
	}

	s <<"----------  ----------  ----------\n";
	return s.str();
}