import bpy
from mathutils import Vector 
import bmesh
import numpy as np
from math import pi
from ..core import composition as core
from ..core import dtype

def toGLM(v):
	if type(v) == Vector:
		return [v.x, v.y, v.z]

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

	m = core.Material()
	m.name = key

	if shader.type == 'EMISSION':
		c = shader.inputs[0].default_value
		s = shader.inputs[1].default_value
		
		m.type = core.MtlType.emit
		m.color = [c[0]*s, c[1]*s, c[2]*s]
		
		return m
		
	elif shader.type == 'BSDF_DIFFUSE':
		c = shader.inputs['Color'].default_value
		
		m.type = core.MtlType.lambert
		m.color = [c[0], c[1], c[2]]
		
		return m
	
	elif shader.type == 'BSDF_GLOSSY':
		c = shader.inputs['Color'].default_value
		a = shader.inputs['Roughness'].default_value
		
		m.type = core.MtlType.glossy
		m.color = [c[0], c[1], c[2]]
		m.a = a*a
		
		return m

	elif shader.type == 'BSDF_GLASS':
		c = shader.inputs['Color'].default_value
		a = shader.inputs['Roughness'].default_value
		ior = shader.inputs['IOR'].default_value

		m.type = core.MtlType.glass
		m.color = [c[0], c[1], c[2]]
		m.a = a*a
		m.ior = ior

		return m

	else:
		print('failed to create a material')

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
		self.data = core.Scene()
		self.mtlBinding = {}

		env = core.Material()
		env.name = 'env'
		env.type = core.MtlType.emit
		env.color = [0, 0, 0]
		self.data.setMaterial(0,env)

	def __str__(self): return self.data.__str__()

	def addMesh(self, key):
		obj = bpy.data.objects[key]

		if not len(obj.material_slots)>0:
			print('no materials')
			return
		
		mesh = getTriangles(obj)
		materialNames = [m.name for m in mesh.materials]

		for name in materialNames:
			if name not in self.mtlBinding.keys():
				self.mtlBinding[name] = self.data.addMaterial(createMaterial(name))
		  
		OW = obj.matrix_world

		vertices = np.array([(
			tuple(OW@v.co),
			tuple( (OW@v.normal - OW.to_translation()).normalized() ))
			for v in mesh.vertices],
			dtype = dtype.Vertex_dtype
		)

		indices = np.array([(
			p.vertices[0], p.vertices[1], p.vertices[2],
			tuple( (OW@p.normal-OW.to_translation()).normalized() ),
			p.use_smooth,
			self.mtlBinding[materialNames[p.material_index]])
			for p in mesh.polygons],
			dtype = dtype.Face_dtype
		)

		self.data.addMesh(vertices, indices, key)
		bpy.data.meshes.remove(mesh)		

	def addSphere(self, key):
		o = bpy.data.objects[key]

		if not len(o.material_slots) > 0:
			print('no materials')
			return
		
		name = o.material_slots[0].name
		if name not in self.mtlBinding.keys():
			self.mtlBinding[name] = self.data.addMaterial(createMaterial(name))

		self.data.addSphere(o.location, sum(o.dimensions)/6, self.mtlBinding[name], key)

	def create(self, spheres, meshes, targets):

		self.setCamera()

		bobjects = bpy.data.objects

		for m in meshes:
			if m in bobjects.keys() and not bobjects[m].hide_get():
				self.addMesh(m)

		for s in spheres:
			if s in bobjects.keys() and not bobjects[s].hide_get():
				self.addSphere(s)

		tids = []
		for t in targets:
			tids.append(self.mtlBinding[t])
		self.data.targetIDs = tids



	def setCamera(self):
		cam = bpy.context.scene.camera
		cam.data.sensor_fit = 'VERTICAL'
		f = 2*cam.data.lens/cam.data.sensor_height
		mat = cam.matrix_world
		self.data.camera.position = mat.to_translation()
		self.data.camera.toWorld = mat.to_3x3()
