#pragma once

#include "Scene.hpp"
#include "Random.hpp"
#include "accel.hpp"

int createScene(Scene*const s);

void pathTracing(glm::vec3*const result, const int w, const int h, const int spp,
	const Scene& scene, RNG*const rngs);

void pathTracing_notTarget(glm::vec3*const result, const int w, const int h, const int spp,
	const Scene& scene, RNG*const rng);

void collectHitpoints(std::vector<hitpoint>& hits,
	const int w, const int h, const int nRay,
	const Scene& scene, RNG& rng);

void collectHitpoints_target_exclusive(std::vector<hitpoint>& hits,
	const uint32_t tMtl, const int targetDepth,
	const int w, const int h, const int nRay,
	const Scene& scene, RNG& rng);

void collectHitpoints_target(std::vector<hitpoint>& hits, 
	const uint32_t tMtl, const int targetDepth,
	const int w, const int h, const int nRay,
	const Scene& scene, RNG& rng);

Tree createPhotonmap(const Scene& scene, const int nPhoton, RNG*const rng, const int nThreads);
void accumulateRadiance(std::vector<hitpoint>& hitpoints, const Tree& photonmap, const Scene& scene, const double alpha);