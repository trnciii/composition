def radiance(hit):
	if hit.iteration > 0:
		return [hit.tau.x/hit.iteration, hit.tau.y/hit.iteration, hit.tau.z/hit.iteration]
	else:
		return [0, 0, 0]

def normal(hit):
	return [hit.n.x, hit.n.y, hit.n.z]

def const(x, y, z):
	def f(hit):
		return [x, y, z]
	return f

def dot(x1, x2):
	return x1[0]*x2[0] + x1[1]*x2[1] + x1[2]*x2[2]

def normalize(v):
	l = dot(v, v)**0.5
	return [v[0]/l, v[1]/l, v[2]/l]

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