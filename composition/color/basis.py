import numpy as np
from .RampData import RampData
from . import basis

## vector helper
def normalize(v):
	return v/np.dot(v, v)**0.5

# hit to vec3

def radiance(hit):
	if hit.iteration > 0:
		return hit.tau/hit.iteration
	else:
		return [0,0,0]

def normal(hit):
	return hit.n

def position(hit):
	return hit.p

def const(x, y, z):
	def f(hit):
		return [x, y, z]
	return f

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
		def f(hit):
			return data.eval(coord(hit))

		return f
	
  # case [[r, g, b], ...]
	if isinstance(data, np.ndarray):
		w = len(data)
		im = np.array(data)

		def f(hit):
			return im[max(0, min(w-1, int(coord(hit)*w)))]
		
		return f

	print('aaaaaa')
	return