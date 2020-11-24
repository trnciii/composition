#pragma once

#include "Scene.hpp"
#include "Random.hpp"
#include "kdtree.hpp"

int createScene(Scene* s);

void renderReference(glm::vec3* const result, const int w, const int h, const int spp, const Scene& scene, RNG* const rngs);
void renderNonTarget(glm::vec3* const result, const int w, const int h, const int spp, const Scene& scene, RNG* const rand);

void collectHitpoints_all(std::vector<hitpoint>& hits,
	const int w, const int h, const int nRay,
	const float R0, const Scene& scene, RNG& rng);

void collectHitpoints_target(std::vector<hitpoint>& hits, const int depth,
	const int w, const int h, const int nRay,
	const float R0, const Scene& scene, const uint32_t target, RNG& rng);

Tree createPhotonmap_all(const Scene& scene, int nPhoton, RNG& rand);
Tree createPhotonmap_target(const Scene& scene, int nPhoton, const uint32_t target, RNG& rand);
void accumulateRadiance(std::vector<hitpoint>& hitpoints, Tree& photonmap, const Scene& scene, const double alpha);