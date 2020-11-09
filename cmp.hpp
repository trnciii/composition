#pragma once

#include "Scene.hpp"
#include "Random.hpp"

int createScene(Scene* s);
void renderReference(glm::vec3* const result, const int w, const int h, const int spp, const Scene& scene, RNG* const rngs);
void renderNonTarget(glm::vec3* const result, const int w, const int h, const int spp, const Scene& scene, RNG* const rand);