import bpy
import math

from .. import core
from .. import color

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

def rampToImage(key, ramp):
    img = bpy.data.images[key]
    w, h = img.size
    print("ramp image size", w, h)
    px = [0.0]*4*w*h
    d = 2**(math.ceil(math.log(ramp.data[-1][0], 2)))

    for i in range(w):
        c = ramp.eval(d*i/w)
        
        for j in range(h):
            idx = j*w + i
            px[4*idx  ] = c.x
            px[4*idx+1] = c.y
            px[4*idx+2] = c.z
            px[4*idx+3] = 1
    
    img.pixels = px