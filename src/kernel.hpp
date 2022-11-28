#pragma once

#include <functional>

#include "Scene.hpp"
#include "Random.hpp"
#include "accel.hpp"
#include "Image.hpp"

std::vector<RNG> createRNGVector(int len, int offset);

int createScene(Scene*const s);

Image PT(int w, int h, const Scene& scene, const int spp, std::vector<RNG>& rng_per_pixel);

Image PT_notTarget(const int w, const int h, const Scene& scene, const int spp, std::vector<RNG>& rng_per_pixel);

std::vector<hitpoint> collectHitpoints(const int w, const int h,
	const int nRay, const Scene& scene, std::vector<RNG>& rng_per_thread);

std::vector<hitpoint> collectHitpoints_target_exclusive
(	const uint32_t tMtl, const int targetDepth,
	const int w, const int h, const int nRay,
	const Scene& scene, std::vector<RNG>& rng_per_thread);

std::vector<hitpoint> collectHitpoints_target
(	const uint32_t tMtl, const int targetDepth,
	const int w, const int h, const int nRay,
	const Scene& scene, std::vector<RNG>& rng_per_thread);

Tree createPhotonmap(const Scene& scene, const int nPhoton, std::vector<RNG>& rng_per_thread);

void progressiveRadianceEstimate(std::vector<hitpoint>& hitpoints, const Tree& photonmap,
	const Scene& scene, const double alpha);

Image PPM(const int w, const int h, const Scene& scene,
	const int nRay, const int nPhoton, const int iteration, const float alpha, const float R0,
	std::vector<RNG>& rng_per_thread);

void radiance_PT(std::vector<hitpoint>& hits, const Scene& scene,
	const int spp, std::vector<RNG>& rng_per_thread);

void radiance_PPM(std::vector<hitpoint>& hits, const Scene& scene,
	const int nPhoton, const int iteration, const float alpha,
	std::vector<RNG>& rngs);

Image nprr(const int w, const int h, const Scene& scene,
	const int spp, std::vector<RNG>& rng_per_pixel,
	const std::vector<std::function<glm::vec3(float)>>& remaps);
