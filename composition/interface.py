import bpy
from . import core

class Context:

	# ppm
	nRay = 128
	R0 = 0.5
	iteration = 100
	nPhoton = 100000
	alpha = 0.6
	eyeDepth = 1

	def __init__(self):
		self.w = 512
		self.h = 512

		self.renderpass = core.RenderPass(self.w, self.h)
		self.bind = {}

		self.scene = core.Scene()
		core.createScene(self.scene);

		self.target1 = 00
		self.target2 = 1

# image
	def bindImage(self, key):
		id = self.renderpass.addLayer()
		self.bind[key] = id
		return id

	def copyImage(self, key):
		bpy.data.images[key].pixels = core.getImage(self.renderpass, self.bind[key])

# rendering
	def renderReference(self, key, spp):
	    core.renderReference(self.renderpass, self.bind[key], spp, self.scene)
	    self.copyImage(key)
	    
	def renderNonTarget(self, key, spp):
	    core.renderNonTarget(self.renderpass, self.bind[key], spp, self.scene)
	    self.copyImage(key)

	def genHits(self, target, nRay):
		return core.collectHitpoints(self.eyeDepth, self.w, self.h, nRay, self.scene, target)

	def ppm(self, target, hits, R0, itr, nPhoton, alpha):
		core.progressivePhotonMapping(hits, R0, itr, nPhoton, alpha, self.scene, target)

	def hitsToImage(self, hits, key, color):
		core.hitsToImage(hits, self.renderpass, self.bind[key], color)
		self.copyImage(key)

	def mask(self, hits, key, nRay):
	    length = self.renderpass.width*self.renderpass.height
	    count = [0]*3*length

	    for i in range(hits.size()):
	        hit = hits.element(i)
	        count[hit.pixel] += 1

	    for i in range(length):    
	        self.renderpass.set(self.bind[key], i, count[i]/nRay, 0, 0)
	    
	    self.copyImage(key)



