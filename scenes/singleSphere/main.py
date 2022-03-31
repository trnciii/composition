import bpy
import time
import math
import numpy as np

import composition
col = composition.color

# define ramps

param = composition.core.PPMParam()
param.nRay = 50
param.R0 = 0.5
param.nPhoton = 50000
param.itr = 10

def target0():
	ramp = 'ColorRamp.001'
	composition.bi.rampToImage(targetMaterials[0]+'_texture', ramp, 256, 16)
	remap = composition.bi.ramp(col.basis.sumRadianceRGB, ramp)

	# return col.basis.radiance

	half = col.basis.const(0.5, 0.5, 0.5)

	remap = col.mul(remap,
		col.add(
			col.mul(half,	col.basis.radiance),
			half
		)
	)

	# return remap

	white = composition.vec3(1,1,1)
	black = composition.vec3(1,0.05,0.05)



	floor = math.floor

	def f(hit):
		p = hit.p*2.5
		return white\
			if pow(-1,floor(p.x)) * pow(-1,floor(p.y)) * pow(-1,floor(p.z)) <0\
			else black

	# e = composition.vec3(0.15)
	return col.mul(f, col.basis.radiance)

targetMaterials = ['sphere']
targetRemap = {'sphere' : target0()}

spheres = ['Sphere.001', 'Sphere']
meshes = ['floor', 'Plane']

def render_2():
	cmp = composition.bi.Context()
	cmp.scene.create(spheres, meshes, targetMaterials)
	cmp.setTargets(targetMaterials)
	print(cmp.scene.data)
	time.sleep(0.1)

	# cmp.pt_ref('pt', 10000)
	# cmp.saveImage('pt')

	# cmp.pt_nt('nt', 10000)
	# cmp.saveImage('nt')
	# print()
	# return

	for n in cmp.targetNames:
		cmp.genHits_ex(n, param.nRay, param.R0)
		cmp.genHits_all(n, param.nRay, param.R0)

	for k, h in cmp.hits.items():
		print(k)

		if k.type is composition.HitsType.EX:
			cmp.radiance_pt(h, 10000)
			# cmp.radiance_ppm(h, param)

		cmp.saveHits(k, param.nRay)

		print()

	cmp.remapAll({n : col.basis.radiance for n in cmp.targetNames})

	print('-- end rendering --')

def remap():
	print('\033[36mremapping\033[0m')

	cmp = composition.bi.Context()
	cmp.setTargets(targetMaterials)
	cmp.loadAll(param.nRay)

	cmp.remapAll(targetRemap)
	cmp.maskAll()

	print('-- end reamapping --')
	return

def render_1():
	cmp = composition.bi.Context()
	cmp.scene.create(spheres, meshes, targetMaterials)
	cmp.setTargets(targetMaterials)
	print(cmp.scene.data)
	time.sleep(0.1)

	nodes = {'sphere' : 'ColorRamp.001'}

	remap = []

	for k, node in nodes.items():
		image = k+'_texture'
		composition.bi.rampToImage(image, node, 256, 16)
		array = np.array(composition.sliceImage(image, 8))
		remap.append((array.flatten(), 0, 10))

	cmp.nprr('nprr', 1000, remap)

	print('-- end nprr --')

render_1()
render_2()
remap()