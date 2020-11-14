#pragma once

#include "Scene.hpp"
#include "Random.hpp"
#include "kdtree.hpp"

int createScene(Scene* s);

void renderReference(glm::vec3* const result, const int w, const int h, const int spp, const Scene& scene, RNG* const rngs);
void renderNonTarget(glm::vec3* const result, const int w, const int h, const int spp, const Scene& scene, RNG* const rand);

void collectHitpoints(std::vector<hitpoint>& hits, int depth,
	const int w, const int h, const int nRay,
	const float R0, const Scene& scene, const uint32_t target, RNG& rng);

void progressivePhotonMapping(std::vector<hitpoint>& hits,
	const float R0, const int iteration, const int nPhoton, const float alpha,
	const Scene& scene, const uint32_t target, RNG& rand);