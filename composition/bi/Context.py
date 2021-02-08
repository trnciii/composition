import bpy
import os

from .. import core
from .Scene import Scene
from .helper import addImage

class Context:

	def __init__(self):
		self.projectName = bpy.path.display_name(bpy.data.filepath).lower()
		self.path = bpy.path.abspath('//') + "result/"
		os.makedirs(self.path, exist_ok=True)
		
		self.files = self.getFiles()

		r_scale = bpy.context.scene.render.resolution_percentage/100
		self.w = int(bpy.context.scene.render.resolution_x * r_scale)
		self.h = int(bpy.context.scene.render.resolution_y * r_scale)
		print('image size', self.w, self.h)

		self.renderpass = core.RenderPass(self.w, self.h)
		self.bind = {}

		self.scene = Scene()

		self.targetNames = []
		self.hits_all = {}
		self.hits_ex = {}
		self.nRay_all = 0
		self.nRay_ex = 0

# image
	def bindImage(self, key):
		id = self.renderpass.addLayer()
		self.bind[key] = id
		return id

	def copyImage(self, key):
		bpy.data.images[key].pixels = core.getImage(self.renderpass, self.bind[key])

	def save(self, key, path):
		if(core.writeLayer(self.renderpass, self.bind[key], path)):
			print("Save layer <", key, ">:", path)
		else:
			print("failed to save a layer")

	def load(self, key, path):
		if core.loadLayer(self.renderpass, self.bind[key], path):
			print("Read a layer:", path)
			self.copyImage(key)
		else:
			print("failed to read a layer")

	def addImages(self, list):
		for t in list:
			addImage(t, self.w, self.h)
			self.bindImage(t)

# target
	def setTargets(self, targetMaterials):
		self.targetNames = targetMaterials
		self.addImages(targetMaterials)
		self.addImages([t+'_mask' for t in targetMaterials])
		self.addImages([t+'_depth' for t in targetMaterials])
		print()


# rendering
	def pt_ref(self, key, spp):
		print('path tracing with sample size', spp)
		core.pt(self.renderpass, self.bind[key], spp, self.scene.data)
		self.copyImage(key)

	def ppm_ref(self, key, param):
		core.ppm(self.renderpass, self.bind[key], param, self.scene.data)
		self.copyImage(key)
		
	def pt_nt(self, key, spp):
		print('path tracing except for targets with sample size', spp)
		core.pt_notTarget(self.renderpass, self.bind[key], spp, self.scene.data)
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

# convert
	def hitsToImage(self, hits, key, color):
		print('(running cpp)')
		core.hitsToImage_cpp(hits, self.renderpass, self.bind[key], color)
		self.copyImage(key)

	def mask(self, hits, key, nRay):
		core.mask(hits, self.renderpass, self.bind[key], nRay)
		self.copyImage(key)

	def depth(self, hits, key, nRay):
		max = core.depth(hits, self.renderpass, self.bind[key], nRay)
		self.copyImage(key)
		print("max depth of", hits , "is", max)
		return max

# more large converter
	def remapAll(self, remaps):
		if not len(remaps) is len(self.targetNames):
			print('failed remapping')
			return

		for i in range(len(self.hits_ex)):
			print('converting\033[33m', self.targetNames[i], '\033[0m', end='')
			self.hitsToImage(self.hits_ex[self.targetNames[i]], self.targetNames[i], remaps[i])
			print()

	def maskAll(self):
		m = [t+'_mask' for t in self.targetNames]
		d = [t+'_depth' for t in self.targetNames]
		
		for i in range(len(self.hits_all)):
			self.mask(self.hits_all[self.targetNames[i]], m[i], self.nRay_all)
			self.depth(self.hits_all[self.targetNames[i]], d[i], self.nRay_all)

		print()

# file
	def getFiles(self):
		files = os.listdir(self.path)
		files.sort()

		print('files in', self.path)
		for f in files:
			print('--',f)
		print()

		return files

	def loadFiles(self, nRay):
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

			if words[0] == 'im' and words[1] == 'nontarget':
				self.load('nt', self.path+file)

			if words[0] == 'im' and words[1] == 'pt':
				self.load('pt', self.path+file)
				
		print()


	def match(self, words, query):
		res = True
		for i in range(len(query)):
			if len(query[i])>0:
				res = res and words[i] == query[i]
		return res