from ..core import vec3

mul = lambda f1, f2: lambda hit: f1(hit) * f2(hit)
add = lambda f1, f2: lambda hit: f1(hit) + f2(hit)
sub = lambda f1, f2: lambda hit: f1(hit) - f2(hit)
div = lambda f1, f2: lambda hit: f1(hit) / f2(hit)
mix = lambda f1, f2, t: lambda hit: (1-t)*f1(hit) + t*f2(hit)

def pow(f1, f2):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		return vec3(c1.x**c2.x, c1.y**c2.y, c1.z**c2.z)
	return g

def max(f1, f2):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		return vec3(c1.x if c1.x>c2.x else c2.x,
			c1.y if c1.y>c2.y else c2.y,
			c1.z if c1.z>c2.z else c2.z,
		)

	return g

def min(f1, f2):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		return vec3(c1.x if c1.x<c2.x else c2.x,
			c1.y if c1.y<c2.y else c2.y,
			c1.z if c1.z<c2.z else c2.z,
		)
	return g