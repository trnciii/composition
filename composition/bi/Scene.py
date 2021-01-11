import bpy
import numpy as np
from ..core import composition
from . import helper

class Scene:
	def __init__(self):
		self.data = composition.Scene()
		self.mtlBinding = {}

	def addMaterial(self, m):
		return self.data.addMaterial(m)

	def addMesh(self, key):
		print('in addMesh')
		if bpy.data.objects[key].type != 'MESH':
			print('not a mesh')
			return
		
		o = bpy.data.objects[key].data

		mtl_names = [m.name for m in o.materials]
		for name in mtl_names:
			if name not in self.mtlBinding.keys():
				mtl = helper.createMaterial(name)
				self.mtlBinding[name] = self.data.addMaterial(mtl)
		
		for i in self.mtlBinding:
			print(i)
		  
		m = bpy.data.objects[key].data
		OW = bpy.data.objects[key].matrix_world

		coes = np.array([[OW @ v.co] for v in m.vertices])
		normals = np.array([[ (OW @ v.normal - OW.to_translation()).normalized() ] for v in m.vertices])

		vertices = np.array([ [coes[i], normals[i]] for i in range(len(m.vertices)) ]).reshape(-1, 6)
		indices = [[p.vertices[0], p.vertices[1], p.vertices[2], self.mtlBinding[o.materials[p.material_index].name]] for p in m.polygons]
		
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