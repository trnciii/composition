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
		o = bpy.data.objects[key]
		if o.type != 'MESH':
			print('not a mesh')
			return

		if not len(o.material_slots)>0:
			print('no materials')
		
		###
		mesh = bpy.data.objects[key].data
		names = [m.name for m in mesh.materials]

		for name in names:
			if name not in self.mtlBinding.keys():
				self.mtlBinding[name] = self.data.addMaterial(helper.createMaterial(name))
		  
		OW = bpy.data.objects[key].matrix_world

		coes = np.array([[OW @ v.co] for v in mesh.vertices])
		normals = np.array([[ (OW @ v.normal - OW.to_translation()).normalized() ] for v in mesh.vertices])

		vertices = np.array([ [coes[i], normals[i]] for i in range(len(mesh.vertices)) ]).reshape(-1, 6)
		indices = [[ p.vertices[0], p.vertices[1], p.vertices[2],\
			self.mtlBinding[names[p.material_index]] ] for p in mesh.polygons]
		
		composition.addMesh(self.data, list(vertices), list(indices))

	def addSphere(self, key):
		o = bpy.data.objects[key]
		if not len(o.material_slots) > 0:
			print('no materials')
			return
		
		###
		name = o.material_slots[0].name
		if name not in self.mtlBinding.keys():
			self.mtlBinding[name] = self.data.addMaterial(helper.createMaterial(name))

		l = o.location
		composition.addSphere(self.data, l.x, l.y, l.z, sum(o.scale)/3, self.mtlBinding[name])

	def setCamera(self, key):
		cam = bpy.data.objects[key]
		cam.data.sensor_fit = 'VERTICAL'
		f = 2*cam.data.lens/cam.data.sensor_height
		mat = sum([list(r) for r in cam.matrix_world], [])
		composition.setCamera(self.data.camera, mat, f)

	def createBoxScene(self):
		composition.createScene(self.data)

	def print(self):
		print('material binding')
		for k in self.mtlBinding.keys():
			print('[{:2}]'.format(self.mtlBinding[k]), k)
		composition.print_scene(self.data)