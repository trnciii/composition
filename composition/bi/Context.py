import bpy
import os
from enum import Enum
from collections import namedtuple

from .. import core
from .Scene import Scene
from .helper import addImage


def quate(s):
	return '\'' + s + '\''


class HitsType(Enum):
	EX = 'ex'
	ALL = 'all'


HitsKey = namedtuple('HitsKey', ['name', 'type', 'nRay'])

def toFilename(key):
	if isinstance(key, HitsKey):
		return "hit_" + key.name +"_" + str(key.nRay) + '_' + key.type.value

def toKey(s):
	if isinstance(s, str):
		words = s.split('_')
		if words[0] == 'hit':
			if words[3] == 'ex' : t = HitsType.EX
			if words[3] == 'all': t = HitsType.ALL

			return HitsKey(words[1], t, int(words[2]))


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

		self.hits = {}

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

# rendering
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

		key = HitsKey(target, HitsType.EX, nRay)
		self.hits[key] = h

	def genHits_all(self, target, nRay, R0):
		print('collecting\033[32m all\033[0m hitpoints on\033[33m', target, '\033[0m')

		depth = 1
		h = core.collectHits_target(self.scene.mtlBinding[target], depth,
			self.w, self.h, nRay, self.scene.data, self.rng_thread)
		
		h.clear(R0)

		key = HitsKey(target, HitsType.ALL, nRay)
		self.hits[key] = h


	def radiance_ppm(self, hit, param):
		print('estimating radiance by\033[32m ppm\033[0m')
		core.radiance_ppm(hit, self.scene.data, param.nPhoton, param.itr, param.alpha, self.rng_thread)

	def radiance_pt(self, hit, spp):
		print('estimating radiance by\033[32m pt\033[0m')
		core.radiance_pt(hit, self.scene.data, spp, self.rng_thread)


# convert
	def hitsToImage(self, hits, key, color):
		# core.hitsToImage_py(hits, self.images[key], color)
		core.hitsToImage_cpp(hits, self.images[key], color)
		self.copyImage(key)

	def mask(self, hits, ikey, nRay):
		image = self.images[ikey]
		count = [0]*len(image.pixels)

		for hit in hits:
			count[hit.pixel] += 1

		for i in range(len(image.pixels)):
			image.pixels[i] = core.vec3(count[i]/nRay, 0, 0)

		self.copyImage(ikey)

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


	def remapAll(self, remaps):
		for k, h in self.hits.items():
			if k.type is HitsType.EX and k.name in remaps.keys():
				print('converting\033[33m', k.name, '\033[0m')
				self.hitsToImage(h, k.name, remaps[k.name])
				print()

	def maskAll(self):
		for k, h in self.hits.items():
			if k.type is HitsType.ALL:
				self.mask(h, k.name+'_mask', k.nRay)
				self.depth(h, k.name+'_depth', k.nRay)

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

	def saveHits(self, key, nRay):
		print(self.hits[key].save(self.path + toFilename(key)))

	def loadAll(self, nRay):
		for file in self.files:
			key = toKey(file)
			if key:
				h = core.Hits()
				h.load(self.path + file)
				self.hits[toKey(file)] = h

			words = file.split('_')

			# if words[0] == 'hit' and words[2] == str(nRay):
			# 	h = core.Hits()
			# 	h.load(self.path+file)

			# 	if words[3] == HitsType.EX.value:
			# 		key = HitsKey(words[1], HitsType.EX, nRay)
			# 		self.hits[key] = h

			# 	if words[3] == HitsType.ALL.value:
			# 		key = HitsKey(words[1], HitsType.ALL, nRay)
			# 		self.hits[key] = h

			if words[0] == 'im':
				self.loadImage(words[1], self.path+file)
				
		print()