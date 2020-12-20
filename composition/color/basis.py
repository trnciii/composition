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

def cel(ramp, p):
    def f(hit):
        l = [p[0]-hit.p.x, p[1]-hit.p.y, p[2]-hit.p.z]
        len = (l[0]*l[0] + l[1]*l[1] + l[2]*l[2])**0.5
        l = [l[0]/len, l[1]/len, l[2]/len]
        dot = l[0]*hit.n.x + l[1]*hit.n.y + l[2]*hit.n.z
        return ramp(0.5*dot + 0.5)
    return f