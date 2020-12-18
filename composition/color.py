from . import core

def radiance(hit):
	if hit.iteration > 0:
	    return [hit.tau.x/hit.iteration, hit.tau.y/hit.iteration, hit.tau.z/hit.iteration]
	else:
		return [0, 0, 0]

def normal(hit):
	return [hit.n.x, hit.n.y, hit.n.z]

def const(hit):
	return [1, 1, 1]