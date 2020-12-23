from ..core import vec3

def mul(f1, f2):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		return vec3(c1.x*c2.x, c1.y*c2.y, c1.z*c2.z)
	return g

def add(f1, f2):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		return vec3(c1.x+c2.x, c1.y+c2.y, c1.z+c2.z)
	return g

def sub(f1, f2):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		return vec3(c1.x-c2.x, c1.y-c2.y, c1.z-c2.z)
	return g

def div(f1, f2):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		return vec3(c1.x/c2.x, c1.y/c2.y, c1.z/c2.z)
	return g

def pow(f1, f2):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		return vec3(c1.x**c2.x, c1.y**c2.y, c1.z**c2.z)
	return g

def mix(f1, f2, t):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		t_ = 1-t
		return vec3(
			t_*c1.x + t*c2.x,
			t_*c1.y + t*c2.y,
			t_*c1.z + t*c2.z
		)

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