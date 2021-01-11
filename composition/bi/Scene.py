import bpy
from ..core import composition
from . import helper

class Scene:
	def __init__(self):
		self.data = composition.Scene()

	def addMaterial(self, m):
		return self.data.addMaterial(m)

	def addMesh(self, vertices, indices):
		composition.addMesh(self.data, list(vertices), list(indices))

	def addSphere_sub(self, x, y, z, r, m:composition.Material):
		composition.addSphere(self.data, float(x), float(y), float(z), float(r), m)

	def addSphere(self, key):
	    o = bpy.data.objects[key]
	    if not len(o.material_slots) > 0 :return

	    m = self.addMaterial(helper.createMaterial(o.material_slots[0].name))
	    l = o.location
	    self.addSphere_sub(l.x, l.y, l.z, sum(o.scale)/3, m)

	def setCamera(self, key):
		cam = bpy.data.objects[key]
		cam.data.sensor_fit = 'VERTICAL'
		f = 2*cam.data.lens/cam.data.sensor_height
		mat = sum([list(r) for r in cam.matrix_world], [])
		composition.setCamera(self.data.camera, mat, f)

	def createBoxScene(self):
		composition.createScene(self.data)

	def print(self):
		composition.print_scene(self.data)