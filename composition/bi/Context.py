import bpy

from .. import core

class Context:

	def __init__(self):
		self.w = 512
		self.h = 512

		self.renderpass = core.RenderPass(self.w, self.h)
		self.bind = {}

		self.scene = core.Scene()
		core.createScene(self.scene);

		self.target1 = 0
		self.target2 = 1

# image
	def bindImage(self, key):
		id = self.renderpass.addLayer()
		self.bind[key] = id
		return id

	def copyImage(self, key):
		bpy.data.images[key].pixels = core.getImage(self.renderpass, self.bind[key])

	def copyAll(self):
		for key in self.bind.keys():
			self.copyImage(key)

	def load(self, key, path):
		if core.loadLayer(self.renderpass, self.bind[key], path):
			print("Read an image")
			print(">>", path)
			self.copyImage(key)

# rendering
	def pt_ref(self, key, spp):
		core.pt(self.renderpass, self.bind[key], spp, self.scene)
		self.copyImage(key)

	def ppm_ref(self, key, param):
		core.ppm(self.renderpass, self.bind[key], param, self.scene)
		self.copyImage(key)
		
	def pt_nt(self, key, spp):
		core.pt_notTarget(self.renderpass, self.bind[key], spp, self.scene)
		self.copyImage(key)

	def genHits_ex(self, target, nRay):
		return core.collectHits_target_exclusive(1, self.w, self.h, nRay, self.scene, target)

	def genHits(self, target, nRay):
		return core.collectHits_target(1, self.w, self.h, nRay, self.scene, target)

	def ppm_radiance(self, hits, target, param):
		core.radiance_target(hits, target, param, self.scene)

# convert
	def hitsToImage(self, hits, key, color):
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