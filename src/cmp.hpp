#pragma once

#include "Scene.hpp"
#include "Random.hpp"
#include "kdtree.hpp"

int createScene(Scene* s);
void renderReference(glm::vec3* const result, const int w, const int h, const int spp, const Scene& scene, RNG* const rngs);
void renderNonTarget(glm::vec3* const result, const int w, const int h, const int spp, const Scene& scene, RNG* const rand);
void collectHitpoints(std::vector<hitpoint>& hits, const int w, const int h, const int nRay, const float R0, const Scene& scene, const uint32_t target, RNG& rng);
Tree createPhotonmap(const Scene& scene, int nPhoton, const uint32_t target, RNG* const rand);
void accumulateRadiance(std::vector<hitpoint>& hitpoints, Tree& photonmap, const Scene& scene, const double alpha);