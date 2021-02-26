import bpy
import os

from .. import core
from .Scene import Scene
from .helper import addImage

def quate(s):
	return '\'' + s + '\''

class Context:

	def __init__(self):
		self.path = bpy.path.abspath('//') + "result/"
		os.makedirs(self.path, exist_ok=True)
		
		self.files = self.getFiles()

		scale = bpy.context.scene.render.resolution_percentage/100
		self.w = int(bpy.context.scene.render.resolution_x * scale)
		self.h = int(bpy.context.scene.render.resolution_y * scale)

		self.images = {}
		self.scene = Scene()
		self.targetNames = []

		threads = os.cpu_count()
		self.rng_thread = core.createRNGs(threads, 0)
		self.rng_pixel = core.createRNGs(self.w*self.h, threads)

		self.hits_all = {}
		self.hits_ex = {}

		self.nRay_all = 0
		self.nRay_ex = 0

		print('cpu count:', threads)
		print('image size:', self.w, self.h)
		print('files in', self.path)
		for f in self.files:
			print('--',f)
		print()

# image
	def bindImage(self, key):
		addImage(key, self.w, self.h)
		self.images[key] = core.Image(self.w, self.h)

	def copyImage(self, key):
		bpy.data.images[key].pixels = self.images[key].toList()

	def setTargets(self, targets):
		self.targetNames = targets
		for t in targets + [t+'_mask' for t in targets] + [t+'_depth' for t in targets]:
			self.bindImage(t)

# ordinary rendering
	def pt_ref(self, key, spp):
		print('path tracing with sample size', spp)
		self.bindImage(key)
		self.images[key] = core.pt(self.w, self.h, self.scene.data, spp, self.rng_pixel)
		self.copyImage(key)

	def ppm_ref(self, key, param):
		print( 'progressive photon mapping:\n'+str(param) )
		self.bindImage(key)
		self.images[key] = core.ppm(self.w, self.h, self.scene.data,
			param.nRay, param.nPhoton, param.itr, param.alpha, param.R0,
			self.rng_thread)
		self.copyImage(key)

	def pt_nt(self, key, spp):
		print('path tracing except for targets with sample size', spp)
		self.bindImage(key)
		self.images[key] = core.pt_notTarget(self.w, self.h, self.scene.data, spp, self.rng_pixel)
		self.copyImage(key)


	def genHits_ex(self, target, nRay, R0):
		print('collecting\033[32m exclusive\033[0m hitpoints on\033[33m', target, '\033[0m')

		depth = 1
		h = core.collectHits_target_exclusive(self.scene.mtlBinding[target], depth,
			self.w, self.h, nRay, self.scene.data, self.rng_thread)
		
		h.clear(R0)

		self.hits_ex[target] = h


	def genHits_all(self, target, nRay, R0):
		print('collecting\033[32m all\033[0m hitpoints on\033[33m', target, '\033[0m')

		depth = 1
		h = core.collectHits_target(self.scene.mtlBinding[target], depth,
			self.w, self.h, nRay, self.scene.data, self.rng_thread)
		
		h.clear(R0)

		self.hits_all[target] = h


	def radiance_ppm(self, hit, param):
		print('estimating radiance by ppm')
		core.radiance_ppm(hit, self.scene.data, param.nPhoton, param.itr, param.alpha, self.rng_thread)

	def radiance_pt(self, hit, spp):
		print('estimating radiance by pt')
		core.radiance_pt(hit, self.scene.data, spp, self.rng_thread)


	def ppm_targets(self, param):
		for target in self.targetNames:
			self.genHits_all(target, param.nRay, param.R0)

			print('estimating radiance by ppm')
			core.radiance_ppm(self.hits_all[target], self.scene.data, param.nPhoton, param.itr, param.alpha, self.rng_thread)
			self.hits_all[target].save(self.path + "hit_" + target +"_" + str(param.nRay) + "_all")

			print()

	def ppm_targets_ex(self, param):
		for target in self.targetNames:
			self.genHits_ex(target, param.nRay, param.R0)

			print('estimating radiance by ppm')
			core.radiance_ppm(self.hits_ex[target], self.scene.data, param.nPhoton, param.itr, param.alpha, self.rng_thread)
			self.hits_ex[target].save(self.path + "hit_" + target + "_" + str(param.nRay) + "_ex")

			print()


	def pt_targets(self, param, spp):
		for target in self.targetNames:
			self.genHits_all(target, param.nRay, param.R0)

			print('estimating radiance by pt')
			core.radiance_pt(self.hits_all[target], self.scene.data, spp, self.rng_thread)
			self.hits_all[target].save(self.path + "hit_" + target +"_" + str(param.nRay) + "_all")

			print()

	def pt_targets_ex(self, param, spp):
		for target in self.targetNames:
			self.genHits_ex(target, param.nRay, param.R0)

			print('estimating radiance by pt')
			core.radiance_pt(self.hits_ex[target], self.scene.data, spp, self.rng_thread)
			self.hits_ex[target].save(self.path + "hit_" + target + "_" + str(param.nRay) + "_ex")

			print()


# convert
	def hitsToImage(self, hits, key, color):
		# core.hitsToImage_py(hits, self.images[key], color)
		core.hitsToImage_cpp(hits, self.images[key], color)
		self.copyImage(key)

	def mask(self, hits, key, nRay):
		image = self.images[key]
		count = [0]*len(image.pixels)

		for hit in hits:
			count[hit.pixel] += 1

		for i in range(len(image.pixels)):
			image.pixels[i] = core.vec3(count[i]/nRay, 0, 0)

		self.copyImage(key)

	def depth(self, hits, key, nRay):
		image = self.images[key]
		d = [0]*len(image.pixels)
		count = [0]*len(image.pixels)
		maxDepth = 0

		for hit in hits:
			d[hit.pixel] += hit.depth
			count[hit.pixel] += 1
			maxDepth = hit.depth if hit.depth > maxDepth else maxDepth
		
		if maxDepth>0:
			for i in range(len(image.pixels)):
				if count[i] > 0:
					image.pixels[i] = core.vec3(d[i]/nRay/maxDepth, 0, 0)

		self.copyImage(key)
		print("max depth of", key , ": ", maxDepth)
		return maxDepth

# more large converter
	def remapAll(self, remaps):
		if not len(remaps) is len(self.targetNames):
			print('failed remapping')
			return

		for i in range(len(self.hits_ex)):
			print('converting\033[33m', self.targetNames[i], '\033[0m')
			self.hitsToImage(self.hits_ex[self.targetNames[i]], self.targetNames[i], remaps[i])
			print()

	def maskAll(self):
		for t in self.targetNames:
			self.mask(self.hits_all[t], t+'_mask', self.nRay_all)
			self.depth(self.hits_all[t], t+'_depth', self.nRay_all)
		print()

# file
	def getFiles(self):
		files = os.listdir(self.path)
		files.sort()
		return files

	def saveImage(self, key):
		path = self.path + 'im_' + key
		if self.images[key].save(path):
			print('Saved', quate(key), 'as', quate(path))
		else:
			print('failed to save', quate(key), 'as', quate(path))

	def loadImage(self, key, path):
		im = core.Image(path)
		if im:
			print("Read an image:", quate(path), 'as', quate(key))
			addImage(key, im.w, im.h)
			self.images[key] = im
			self.copyImage(key)
		else:
			print("failed to read an image", quate(path))

	def loadAll(self, nRay):
		for file in self.files:
			words = file.split('_')
			if words[0] == 'hit' and words[2] == str(nRay):
				h = core.Hits()
				h.load(self.path+file)

				if words[3] == 'ex':
					self.nRay_ex = nRay
					self.hits_ex[words[1]] = h

				if words[3] == 'all':
					self.nRay_all = nRay
					self.hits_all[words[1]] = h

			if words[0] == 'im':
				self.loadImage(words[1], self.path+file)
				
		print()


	def match(self, words, query):
		res = True
		for i in range(len(query)):
			if len(query[i])>0:
				res = res and words[i] == query[i]
		return res