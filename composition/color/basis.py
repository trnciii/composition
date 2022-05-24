import numpy as np
from .RampData import RampData
from . import basis

## vector helper
def normalize(v):
	return v/np.sqet(np.dot(v, v))

# hit to vec3

def radiance(hit):
	return hit.tau/hit.iteration if hit.iteration>0 else np.zeros(3)

def normal(hit):
	return hit.n

def position(hit):
	return hit.p

def const(x, y, z):
	ret = np.array([x, y, z])
	return lambda hit: ret

def cel_diffuse(ramp, p):
	def f(hit):
		d = np.dot(normalize(p-hit.p), hit.n)
		return ramp.eval(0.5*d + 0.5)
	return f

def cel_specular(ramp, p):
	def f(hit):
		r = 2*hit.n*np.dot(hit.n, hit.wo) - hit.wo
		d = np.dot(normalize(p-hit.p), r)
		return ramp.eval(0.5*d + 0.5)
	return f

def sumRadianceRGB(hit):
	return np.sum(radiance(hit))

def ramp(coord, data):
	if isinstance(data, RampData):
		ev = data.eval
		return lambda hit: ev(coord(hit))
	
  # case [[r, g, b], ...]
	if isinstance(data, np.ndarray):
		w = data.shape[0]
		return lambda hit: data[max(0, min(w-1, int(coord(hit)*w)))]

	print('aaaaaa')
	return