#pragma once

#include <algorithm>
#include <glm/glm.hpp>
#include <vector>
#include "data.hpp"

struct Box{
	glm::vec3 min;
	glm::vec3 max;

	inline void init(const glm::vec3& p){
		min = p;
		max = p;
	}

	inline void update(const glm::vec3& p){
		min = glm::min(min, p);
		max = glm::max(max, p);
	}

	inline int axis(){
		glm::vec3 dim = max - min;
		return (dim.y<dim.x && dim.z<dim.x)? 0 : ((dim.z<dim.y)? 1 : 2);
	}

	inline bool intersect(const glm::vec3& p, float r){
		return (min.x - r < p.x && p.x < max.x + r
			&& min.y - r < p.y && p.y < max.y + r
			&& min.z - r < p.z && p.z < max.z + r);
	}

	bool dist_sub(float o, float d, float min, float max, float w[2])const;
	float distance(const Ray &ray)const;
};

struct Tree{

	struct Node{
		Box box;

		const uint32_t begin;
		const uint32_t size;
		uint32_t next;

		inline Node(const Tree* const tree, const uint32_t b, const uint32_t e)
		:size(e-b), next(0), begin(b)
		{
			if(size<1)return;
			box.init(tree->verts[begin].p);
			for(int i=begin+1; i<begin + size; i++)box.update(tree->verts[i].p);
		}
	};

	struct Result{
		float distance;
		Photon photon;

		Result(Photon p, float d):distance(d), photon(p){}

		static bool compareDistance(const Result& a, const Result& b){return a.distance < b.distance;}
	};


private:

	std::vector<Photon> verts;
	std::vector<Node> nodes;
	const uint32_t nElements = 1000;

	void split(const uint32_t verts_begin, const uint32_t verts_end, const int axis);

public:

	bool build();
	std::vector<Result> searchNN(const hitpoint& hit);
	std::vector<Result> searchNN_checkAll(const hitpoint& hit);
	void copyElements(Photon* const elements, uint32_t size);
	void addElements(Photon* const elements, uint32_t size);
	inline bool hasTree(){return 0 < nodes.size();}
	inline std::vector<Node> getNodes(){return nodes;}
	inline std::vector<Photon> getElements(){return verts;}

};


#ifdef IMPLEMENT_TREE

////////////
// BOX
////////////

float Box::distance(const Ray &ray)const{
	float tx[2], ty[2], tz[2], t[2];
	if(dist_sub(ray.o.x, ray.d.x, min.x, max.x, tx)
		&& dist_sub(ray.o.y, ray.d.y, min.y, max.y, ty)
		&& dist_sub(ray.o.z, ray.d.z, min.z, max.z, tz)
		){
		t[0] = (tx[0]<ty[0]) ?ty[0] :tx[0]; //get max
		if(t[0]<tz[0]) t[0]=tz[0];

		t[1] = (tx[1]<ty[1]) ?tx[1] :ty[1]; //get min
		if(tz[1]<t[1]) t[1]=tz[1];

		if(t[0]<=t[1])  return(0<t[0]) ?t[0]:t[1];
	}
	return -1;
}

bool Box::dist_sub(float o, float d, float min, float max, float w[2])const{
	if(d==0) return min<o && o<max;

	float temp[2] = {(max - o)/d, (min - o)/d};
	if(temp[0]<temp[1]){
		w[0] = temp[0];
		w[1] = temp[1];
	}
	else{
		w[0] = temp[1];
		w[1] = temp[0];
	}
	return true;
}

////////////
// TREE
////////////

void Tree::split(const uint32_t begin, const uint32_t end, const int axis){
	std::sort(verts.begin()+begin, verts.begin()+end,
		[axis](Photon a, Photon b){return a.p[axis] < b.p[axis];});	
	const uint32_t mid =  begin+(end-begin)/2; // split by count

	uint32_t p0 = nodes.size();
	{
		Node node(this, begin, mid);
		nodes.push_back(node);
		if(nElements < node.size)split(begin, mid, node.box.axis());
	}
	
	uint32_t p1 = nodes.size();
	{
		Node node(this, mid, end);
		nodes.push_back(node);
		if(nElements < node.size)split(mid, end, node.box.axis());
	}

	uint32_t p2 = nodes.size();

	nodes[p0].next = p1 - p0;
	nodes[p1].next = p2 - p1;
};

bool Tree::build(){
	nodes.clear();

	if(verts.size() < 1) return false;
	
	Node root(this, 0, verts.size());
	nodes.push_back(root);
	if(nElements < root.size) split(0, verts.size(), root.box.axis());
	nodes[0].next = nodes.size();
	return true;
}

std::vector<Tree::Result> Tree::searchNN(const hitpoint& hit){
	if(!hasTree()) return searchNN_checkAll(hit);

	std::vector<Tree::Result> result;

	uint32_t i = 0;
	while(i < nodes.size()){
		if(nodes[i].box.intersect(hit.p, hit.R)){
			if(nodes[i].size <= nElements)
				for(int j=nodes[i].begin; j<nodes[i].begin + nodes[i].size; j++){
					Photon& photon = verts[j];
					glm::vec3 d = photon.p - hit.p;
					float l = length(d);
					d /= l;

					if(l < hit.R /*&& dot(hit.n, d) < hit.R*hit.R*0.01*/)
						result.push_back(Tree::Result(photon, l));
				}
			i++;
		}
		else i += nodes[i].next;
	}

	return result;
}

std::vector<Tree::Result> Tree::searchNN_checkAll(const hitpoint& hit){
	std::vector<Tree::Result> result;
	
	for(auto v : verts){
		float d = length(v.p-hit.p);
		if(d < hit.R)
			result.push_back(Tree::Result(v, d));
	}

	return result;
}

void Tree::copyElements(Photon* const elements, uint32_t size){
	std::vector<Photon> v(elements, elements+size);
	verts.swap(v);
	nodes.clear();
}

void Tree::addElements(Photon* const elements, uint32_t size){
	std::vector<Photon> v(elements, elements+size);
	verts.reserve(verts.size()+size);
	std::copy(v.begin(), v.end(), back_inserter(verts));
	nodes.clear();
}


#endif
