import bpy
import time
import numpy as np
import composition
col = composition.color    

def validateGlobals():
	req = ['scene', 'targetMaterials', 'nt']

	for r in req:
		if not r in globals():
			print('Could not find', r)
			return False

	return True


def background(cmp):
	#print('pt'); #cmp.pt_ref(rf, 500)
	#print('ppm ref'); cmp.ppm_ref(rf, param)

	print('pt_nt');
	cmp.pt_nt(nt, 1000)
	cmp.save(nt, cmp.path+"im_nontarget")
	print('')

def ppm_targets_ex(cmp, param):
	res = []
	for i in range(len(cmp.scene.data.targets)):
		print('collecting\033[32m exclusive\033[0m hitpoints on\033[33m', targetMaterials[i], '\033[0m')
		hits = cmp.genHits_ex(i, param.nRay)

		print('estimating radiance')
		cmp.ppm_radiance(hits, i, param)
		hits.save(cmp.path + "hit_" + targetMaterials[i] + "_" + str(param.nRay) + "_ex")

		res.append(hits)
		print()

	return res

def ppm_targets(cmp, param):
	res = []
	for i in range(len(cmp.scene.data.targets)):
		print('collecting\033[32m all\033[0m hitpoints on\033[33m', targetMaterials[i], '\033[0m')
		hits = cmp.genHits(i, param.nRay)

		print('estimating radiance')
		cmp.ppm_radiance(hits, i, param)
		hits.save(cmp.path + "hit_" + targetMaterials[i] +"_" + str(param.nRay) + "_all")

		res.append(hits)
		print()

	return res

def render():
	cmp = composition.bi.Context()

	cmp.bindImage(nt)
	cmp.addImages(targetMaterials)
	
	scene(cmp.scene)
	for t in targetMaterials:
		cmp.scene.addTarget(t)
	cmp.scene.print()
	time.sleep(0.1)

	background(cmp)

	hits_all = ppm_targets(cmp, param)
	hits_ex = ppm_targets_ex(cmp, param)

	for i in range(len(hits_ex)):
		print('converting\033[33m', targetMaterials[i], '\033[0m', end='')
		cmp.hitsToImage(hits_ex[i], targetMaterials[i], col.basis.radiance)
		print()
	return


print('\033[36mrender.py\033[0m')
if validateGlobals():
	render()
print('---- end of render.py ----\n')