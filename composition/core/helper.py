from . import composition

class PPMParam:
	nRay = 8
	nPhoton = int(1e6)
	iteration = 10000
	alpha = 0.6
	R0 = 1.0

def ppm(renderpass, id, param, scene):
	composition.ppm_ref(renderpass, id, param.nRay, param.nPhoton, param.iteration, param.alpha, param.R0, scene)