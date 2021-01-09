from . import composition

class PPMParam:
	nRay = 8
	nPhoton = 10000
	iteration = 10
	alpha = 0.6
	R0 = 1.0

def ppm(renderpass, id, param, scene):
	composition.ppm(renderpass, id, param.nRay, param.nPhoton, param.iteration, param.alpha, param.R0, scene)