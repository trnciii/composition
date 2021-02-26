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

		self.hits_all = {}
		self.hits_ex = {}

		self.nRay_all = 0
		self.nRay_ex = 0

		print('image size', self.w, self.h)
		print('files in', self.path)
		for f in self.files:
			print('--',f)
		print()

# image
	def bindImage(self, key):
		addImage(key, self.w, self.h)
		self.images[key] = core.Image(self.w, self.h)

	def copyImage(self, key):
		bpy.data.images[key].pixels = core.getImage(self.images[key])

	def setTargets(self, targets):
		self.targetNames = targets
		for t in targets + [t+'_mask' for t in targets] + [t+'_depth' for t in targets]:
			self.bindImage(t)

# rendering
	def pt_ref(self, key, spp):
		print('path tracing with sample size', spp)
		self.bindImage(key)		
		core.pt(self.images[key], spp, self.scene.data)
		self.copyImage(key)

	def ppm_ref(self, key, param):
		print( 'progressive photon mapping:\n'+str(param) )
		self.bindImage(key)
		core.ppm(self.images[key], param, self.scene.data)
		self.copyImage(key)
		
	def pt_nt(self, key, spp):
		print('path tracing except for targets with sample size', spp)
		self.bindImage(key)
		core.pt_notTarget(self.images[key], spp, self.scene.data)
		self.copyImage(key)

	def genHits_ex(self, target, nRay):
		return core.collectHits_target_exclusive(1, self.w, self.h, nRay, self.scene.data, self.scene.mtlBinding[target])

	def genHits(self, target, nRay):
		return core.collectHits_target(1, self.w, self.h, nRay, self.scene.data, self.scene.mtlBinding[target])

	def ppm_radiance(self, hits, target, param):
		core.radiance_target(hits, self.scene.mtlBinding[target], param, self.scene.data)

# more large rendering processes
	def ppm_target(self, target, param):
		print('collecting\033[32m all\033[0m hitpoints on\033[33m', target, '\033[0m')
		hits = self.genHits(target, param.nRay)

		print('estimating radiance')
		self.ppm_radiance(hits, target, param)

		hits.save(self.path + "hit_" + target +"_" + str(param.nRay) + "_all")
		self.hits_all[target] = hits
		print()

	def ppm_target_ex(self, target, param):
		print('collecting\033[32m exclusive\033[0m hitpoints on\033[33m', target, '\033[0m')
		hits = self.genHits_ex(target, param.nRay)

		print('estimating radiance')
		self.ppm_radiance(hits, target, param)

		hits.save(self.path + "hit_" + target + "_" + str(param.nRay) + "_ex")
		self.hits_ex[target] = hits
		print()

	def ppm_targets(self, param):
		res = {}
		for target in self.targetNames:
			print('collecting\033[32m all\033[0m hitpoints on\033[33m', target, '\033[0m')
			hits = self.genHits(target, param.nRay)

			print('estimating radiance')
			self.ppm_radiance(hits, target, param)
			hits.save(self.path + "hit_" + target +"_" + str(param.nRay) + "_all")

			res[target] = hits
			print()

		self.hits_all = res

	def ppm_targets_ex(self, param):
		res = {}
		for target in self.targetNames:
			print('collecting\033[32m exclusive\033[0m hitpoints on\033[33m', target, '\033[0m')
			hits = self.genHits_ex(target, param.nRay)

			print('estimating radiance')
			self.ppm_radiance(hits, target, param)
			hits.save(self.path + "hit_" + target + "_" + str(param.nRay) + "_ex")

			res[target] = hits
			print()

		self.hits_ex = res

	def pt_targets(self, param, spp):
		res = {}
		for target in self.targetNames:
			print('collecting\033[32m all\033[0m hitpoints on\033[33m', target, '\033[0m')
			hits = self.genHits(target, param.nRay)

			print('estimating radiance')
			core.radiance_pt(hits, self.scene.data, spp)
			hits.save(self.path + "hit_" + target +"_" + str(param.nRay) + "_all")

			res[target] = hits
			print()

		self.hits_all = res

	def pt_targets_ex(self, param, spp):
		res = {}
		for target in self.targetNames:
			print('collecting\033[32m exclusive\033[0m hitpoints on\033[33m', target, '\033[0m')
			hits = self.genHits_ex(target, param.nRay)

			print('estimating radiance')
			core.radiance_pt(hits, self.scene.data, spp)
			hits.save(self.path + "hit_" + target + "_" + str(param.nRay) + "_ex")

			res[target] = hits
			print()

		self.hits_ex = res

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