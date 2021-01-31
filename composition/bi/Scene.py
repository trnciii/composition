import bpy
import bmesh
import numpy as np
from math import pi
from ..core import composition

def findShaders(key):
	tree = bpy.data.materials[key].node_tree
	links = tree.links
	nodes = tree.nodes
	names = [n.name for n in nodes]
	
	shaders = [] 
	for l in links:
		t = l.to_node
		f = l.from_node
		if type(t) is bpy.types.ShaderNodeOutputMaterial:
			if t.is_active_output and l.to_socket.name == 'Surface':
				shaders.append(f)
	
	return shaders

def createMaterial(key):
	shaders = findShaders(key)

	if not len(shaders)>0:
		print('could not convert material')
		return

	shader = shaders[0]

	if shader.type == 'EMISSION':
		c = shader.inputs[0].default_value
		s = shader.inputs[1].default_value
		
		m = composition.Material()
		m.type = composition.MtlType.emit
		m.color = composition.vec3(c[0]*s, c[1]*s, c[2]*s)
		
		return m
		
	elif shader.type == 'BSDF_DIFFUSE':
		c = shader.inputs['Color'].default_value
		
		m = composition.Material()
		m.type = composition.MtlType.lambert
		m.color = composition.vec3(c[0], c[1], c[2])
		
		return m
	
	elif shader.type == 'BSDF_GLOSSY':
		c = shader.inputs['Color'].default_value
		a = shader.inputs['Roughness'].default_value
		
		m = composition.Material()
		m.type = composition.MtlType.glossy
		m.color = composition.vec3(c[0], c[1], c[2])
		m.a = a*a
		
		return m

	elif shader.type == 'BSDF_GLASS':
		c = shader.inputs['Color'].default_value
		a = shader.inputs['Roughness'].default_value
		ior = shader.inputs['IOR'].default_value

		m = composition.Material()
		m.type = composition.MtlType.glass
		m.color = composition.vec3(c[0], c[1], c[2])
		m.a = a*a
		m.ior = ior

		return m

def getTriangles(obj):
	depsgraph = bpy.context.evaluated_depsgraph_get()
	object_eval = obj.evaluated_get(depsgraph)

	mesh = bpy.data.meshes.new_from_object(object_eval)

	bm = bmesh.new()
	bm.from_mesh(mesh)

	bmesh.ops.triangulate(bm, faces=bm.faces[:])

	bm.to_mesh(mesh)
	bm.free

	return mesh

class Scene:
	def __init__(self):
		self.data = composition.Scene()
		self.mtlBinding = {}

		env = composition.Material()
		env.type = composition.MtlType.emit
		env.color = composition.vec3(0, 0, 0)
		self.setEnvironment(env)

	def setEnvironment(self, m):
		composition.setEnvironment(self.data, m)

	def addMesh(self, key):
		obj = bpy.data.objects[key]

		if not len(obj.material_slots)>0:
			print('no materials')
			return
		
		###
		mesh = getTriangles(obj)
		names = [m.name for m in mesh.materials]

		for name in names:
			if name not in self.mtlBinding.keys():
				self.mtlBinding[name] = self.data.addMaterial(createMaterial(name))
		  
		OW = obj.matrix_world

		coes = np.array([[OW @ v.co] for v in mesh.vertices])
		normals = np.array([[ (OW @ v.normal - OW.to_translation()).normalized() ] for v in mesh.vertices])

		vertices = np.array([ [coes[i], normals[i]] for i in range(len(mesh.vertices)) ]).reshape(-1, 6)
		indices = [[ p.vertices[0], p.vertices[1], p.vertices[2],\
			self.mtlBinding[names[p.material_index]] ] for p in mesh.polygons]
		
		composition.addMesh(self.data, list(vertices), list(indices))
		bpy.data.meshes.remove(mesh)		

	def addSphere(self, key):
		o = bpy.data.objects[key]

		if not len(o.material_slots) > 0:
			print('no materials')
			return
		
		name = o.material_slots[0].name
		if name not in self.mtlBinding.keys():
			self.mtlBinding[name] = self.data.addMaterial(createMaterial(name))

		l = o.location
		composition.addSphere(self.data, l.x, l.y, l.z, sum(o.dimensions)/6, self.mtlBinding[name])

	def create(self, spheres, meshes, targets):

		self.setCamera()

		bobjects = bpy.data.objects

		for m in meshes:
			if m in bobjects.keys() and not bobjects[m].hide_get():
				self.addMesh(m)

		for s in spheres:
			if s in bobjects.keys() and not bobjects[s].hide_get():
				self.addSphere(s)

		for t in targets:
			self.data.targets.append(self.mtlBinding[t])
			

	def setCamera(self):
		cam = bpy.context.scene.camera
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
		print()
		composition.print_scene(self.data)