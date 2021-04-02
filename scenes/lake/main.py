import bpy
import time
import composition
col = composition.color


param_preview = composition.core.PPMParam()
param_preview.nRay = 16
param_preview.nPhoton = 1000
param_preview.itr = 10

param_final = composition.core.PPMParam()
param_final.nRay = 16
param_final.nPhoton = 1000000
param_final.itr = 100000

param = param_preview

def target0():
	ramp = 'ColorRamp'
	
	def u(hit):
		return col.basis.sumRadianceRGB(hit)*0.5

	composition.bi.rampToImage(targetMaterials[0]+'_texture', ramp, 256, 16)
	remap = composition.bi.ramp(u, ramp)

	return remap

def target1():
	ramp = 'ColorRamp.001'
	
	def u(hit):
		return col.basis.sumRadianceRGB(hit)*5
	
	composition.bi.rampToImage(targetMaterials[1]+'_texture', ramp, 256, 16)
	remap = composition.bi.ramp(u, ramp)
	return remap

def target2():
	ramp = 'ColorRamp.002'
	
	def u(hit):
		return col.basis.sumRadianceRGB(hit)
	
	composition.bi.rampToImage(targetMaterials[1]+'_texture', ramp, 256, 16)
	remap = composition.bi.ramp(u, ramp)
	return remap


targetMaterials = ['diffuse', 'water', 'earth']
targetRemap = {
	'diffuse' : target0(),
	'water' : target1(),
	'earth' : target2()
}

spheres = [
	'sky',
	'source',
]
meshes = [
	'Suzanne',
	'earth',
	'water',
	'Plane',
	'Plane.002',
]

def render():
	print('\033[36mrendering\033[0m')
	
	cmp = composition.bi.Context()
	cmp.scene.create(spheres, meshes, targetMaterials)
	cmp.setTargets(targetMaterials)
	print(cmp.scene)
	time.sleep(0.1)

	cmp.pt_ref('pt', 2000)
	cmp.saveImage('pt')

	cmp.pt_nt('nt', 2000)
	cmp.saveImage('nt')
	print()    

	for t in cmp.targetNames:
		cmp.genHits_ex(t, param.nRay, param.R0)
		cmp.genHits_all(t, param.nRay, param.R0)

	print()

	for k, h in cmp.hits.items():
		print(k)

		if k.type is composition.HitsType.EX:
			cmp.radiance_pt(h, 10000)
			# cmp.radiance_ppm(h, param)
		
		cmp.saveHits(k, param.nRay)

		print()


	cmp.remapAll({n : col.basis.radiance for n in cmp.targetNames})

	print('-- end rendering --')
	return

def remap():
	print('\033[36mremapping\033[0m')

	cmp = composition.bi.Context()
	cmp.setTargets(targetMaterials)
	cmp.loadAll(param.nRay)

	cmp.remapAll(targetRemap)
	cmp.maskAll()

	print('-- end reamapping --')
	return

render()
remap()