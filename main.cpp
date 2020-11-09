#include "cmp.hpp"

#include <filesystem>
#include <cassert>
#include <iostream>

#include "RenderPasses.hpp"
#include "Random.hpp"
#include "print.hpp"
#include "file.hpp"

int main(void){
	// create output directory.
	// using string for dir-name to later create output filename
	// because the format of path::c_str depends on OS.
	std::string outDir("result");
	if(!( std::filesystem::exists(outDir) && std::filesystem::is_directory(outDir) )){
		std::cout <<"mkdir " <<outDir <<std::endl;
		printBr();
		assert(std::filesystem::create_directory(outDir));
	}

	std::string outDir_p = outDir + "/progress";
	if(!( std::filesystem::exists(outDir_p) && std::filesystem::is_directory(outDir_p) )){
		std::cout <<"mkdir " <<outDir_p <<std::endl;
		printBr();
		assert(std::filesystem::create_directory(outDir_p));
	}
	
	Scene scene;
	createScene(&scene);
	print(scene);

	RNG rand;
	int width = 512;
	int height = 512;

	RenderPasses passes(width, height);

	// render parameters
	int nIteration = 10000;
	int outInterval = 100;

	int nPhoton = 10000;
	float initialRadius = 1;
	int nRay = 16;
	float alpha = 0.7;
	int spp_pt = 1000;

	if(scene.aggregationTarget.size()==0){
		puts("no target");
		return 0;
	}
	uint32_t aggregationTarget = scene.aggregationTarget[0];


	int reference = passes.addLayer();
	{
		std::cout <<"path tracing for reference..." <<std::endl;

		RNG* rngForEveryPixel = new RNG[width*height];
		for(int i=0; i<width*height; i++)
			rngForEveryPixel[i] = RNG(i);

		renderReference(passes.data(reference), width, height, 100, scene, rngForEveryPixel);
		
		if(writeImage(passes.data(reference), width, height, (outDir + "/reference.png").data()) == 1)
			std::cout <<" reference saved" <<std::endl;
		else std::cout <<"failed to save image" <<std::endl;

		delete[] rngForEveryPixel;
	}


	int non_target = passes.addLayer();
	{
		std::cout <<"path tracing for non-target component..." <<std::endl;

		RNG* rngForEveryPixel = new RNG[width*height];
		for(int i=0; i<width*height; i++)
			rngForEveryPixel[i] = RNG(i);

		renderNonTarget(passes.data(non_target), width, height, 100, scene, rngForEveryPixel);
		
		if(writeImage(passes.data(non_target), width, height, (outDir + "/non-target.png").data()) == 1)
			std::cout <<" non-target saved" <<std::endl;
		else std::cout <<"failed to save image" <<std::endl;

		delete[] rngForEveryPixel;
	}


	// if(result_non.write(outDir + "images"))std::cout <<"images saved" <<std::endl;
	

	// RenderResult result(width, height);
	// if(result.read(outDir + "/images"))std::cout <<"non target component load" <<std::endl;
	// saveResult(result, RenderResult::Layer::NONTARGET, outDir);
	// printRule();


	// std::vector<hitpoint> hits;
	// hits.reserve(width*height*nRay);

	// std::cout <<"collecting hitpoints for target component..." <<std::endl;
	// for(int i=0; i<result.length*nRay; i++){
	// 	int idx = i/nRay;
	// 	int xi = idx%result.width;
	// 	int yi = idx/result.width;

	// 	double x = (double) (2*(xi+rand.uniform())-result.width )/result.height;
	// 	double y = (double)-(2*(yi+rand.uniform())-result.height)/result.height;
	// 	Ray ray = scene.camera.ray(x, y);
	// 	vec3 throuput(1);
	// 	double pDepth = 1;

	// 	while(rand.uniform()<pDepth){
	// 	// for(int depth=0; depth<5; depth++){
	// 		Intersection is = intersect(ray, scene);
	// 		Material& mtl = scene.materials[is.mtlID];

	// 		if(mtl.type == Material::Type::EMIT) break;

	// 		if(is.mtlID == aggregationTarget ){
	// 			double p = 0.5;
	// 			if(rand.uniform()<p){
	// 				throuput /= p;
	// 				hits.push_back(hitpoint(is, initialRadius, throuput/nRay, idx, ray));
	// 				result.data(RenderResult::Layer::DISTRIBUTION)[idx] += throuput/nRay;
	// 				break;
	// 			}
	// 			else throuput /= (1-p);
	// 		}


	// 		vec3 tan[2];
	// 		tangentspace(is.n, tan);
	// 		vec3 hemi = sampleCosinedHemisphere(rand.uniform(), rand.uniform());

	// 		ray.o = offset(is.p, is.n);
	// 		ray.d = (is.n*hemi.z) + (tan[0]*hemi.x) + (tan[1]*hemi.y);

	// 		throuput *= mtl.color/pDepth;
	// 		pDepth = std::min(max(throuput), 1.0);
	// 	}
	// }
	// saveResult(result, RenderResult::Layer::DISTRIBUTION, outDir);
	// printRule();

	// if(writeVector(hits, outDir + "/hit"))std::cout <<"hitpoints saved" <<std::endl;

	// std::vector<hitpoint> hit_target;
	// if(readVector(hit_target, outDir + "/hit"))std::cout <<"hitpoints load" <<std::endl;


	// std::cout <<"progressive estimation pass for target component..." <<std::endl;
	// printBr();
	// for(int iteration=1; iteration<=nIteration; iteration++){
	// 	Tree photonmap = createPhotonmap(scene, nPhoton, aggregationTarget, &rand);
	// 	accumulateRadiance(hit_target, photonmap, scene, alpha);

	// 	if(iteration%outInterval == 0 || iteration == nIteration){
	// 		std::cout <<"composing an image" <<std::endl;
	// 		std::cout <<"itr = " <<iteration <<std::endl;

	// 		composeImage(result, iteration, hit_target);
			
	// 		char out_target[64];
	// 		sprintf(out_target, "targat_%04d.png", iteration);

	// 		char out_comp[64];
	// 		sprintf(out_comp, "composed_%04d.png", iteration);

	// 		saveResult(result, RenderResult::Layer::TARGET, outDir_p, out_target);
	// 		saveResult(result, RenderResult::Layer::COMPOSED, outDir_p, out_comp);
	// 		saveResult(result, RenderResult::Layer::TARGET, outDir);
	// 		saveResult(result, RenderResult::Layer::COMPOSED, outDir);
		
	// 		printRule();
	// 	}

	// }

	return 0;
}