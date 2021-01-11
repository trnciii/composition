import bpy
from ..core import composition

class Scene:
	def __init__(self):
		self.data = composition.Scene()

	def addMaterial(self, m):
		return self.data.addMaterial(m)

	def addMesh(self, vertices, indices):
		composition.addMesh(self.data, list(vertices), list(indices))

	def addSphere(self, x, y, z, r, m:composition.Material):
		composition.addSphere(self.data, float(x), float(y), float(z), float(r), m)

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