from ..core import vec3
from .RampData import RampData
from . import basis

## instant helper
def dot(x1, x2):
	return x1.x*x2.x + x1.y*x2.y + x1.z*x2.z

def normalize(v):
	l = dot(v, v)**0.5
	return vec3(v.x/l, v.y/l, v.z/l)

# hit to vec3

def radiance(hit):
	if hit.iteration > 0:
		return vec3(hit.tau.x/hit.iteration, hit.tau.y/hit.iteration, hit.tau.z/hit.iteration)
	else:
		return vec3(0,0,0)

def normal(hit):
	return hit.n

def const(x, y, z):
	def f(hit):
		return vec3(x, y, z)
	return f

def cel_diffuse(ramp, p):
	def f(hit):
		l = normalize([p[0]-hit.p.x, p[1]-hit.p.y, p[2]-hit.p.z])
		d = dot(l, [hit.n.x, hit.n.y, hit.n.z])
		return ramp(0.5*d + 0.5)
	return f

def cel_specular(ramp, p):
	def f(hit):
		l = normalize([p[0]-hit.p.x, p[1]-hit.p.y, p[2]-hit.p.z])
		v = [hit.wo.x, hit.wo.y, hit.wo.z]
		m = normalize([l[0]+v[0], l[1]+v[1], l[2]+v[2]])
		d = dot(m, [hit.n.x, hit.n.y, hit.n.z])
		return ramp(0.5*d + 0.5)
	return f

def sumRadianceRGB(hit):
	tau = radiance(hit)
	return tau.x + tau.y + tau.z

def ramp(coord, data):
	if type(data) is RampData:
		def f(hit):
			return data.eval(coord(hit))

		return f
	
	if type(data) is list:
		w = len(data)
		im = [vec3(p[0], p[1], p[2]) for p in data]

		def f(hit):
			return im[max(0, min(w-1, int(coord(hit)*w)))]
		
		return f

	return