def mul(f1, f2):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		return [c1[0]*c2[0], c1[1]*c2[1], c1[2]*c2[2]]
	return g

def add(f1, f2):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		return [c1[0]+c2[0], c1[1]+c2[1], c1[2]+c2[2]]
	return g

def sub(f1, f2):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		return [c1[0]-c2[0], c1[1]-c2[1], c1[2]-c2[2]]
	return g

def div(f1, f2):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		return [c1[0]/c2[0], c1[1]/c2[1], c1[2]/c2[2]]
	return g

def pow(f1, f2):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		return [c1[0]**c2[0], c1[1]**c2[1], c1[2]**c2[2]]
	return g

def mix(f1, f2, t):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		t_ = 1-t
		return [
			t_*c1[0] + t*c2[0],
			t_*c1[1] + t*c2[1],
			t_*c1[2] + t*c2[2]
		]

	return g

def max(f1, f2):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		return [c1[0] if c1[0]>c2[0] else c2[0],
			c1[1] if c1[1]>c2[1] else c2[1],
			c1[2] if c1[2]>c2[2] else c2[2]]

	return g

def min(f1, f2):
	def g(hit):
		c1 = f1(hit)
		c2 = f2(hit)
		return [c1[0] if c1[0]<c2[0] else c2[0],
			c1[1] if c1[1]<c2[1] else c2[1],
			c1[2] if c1[2]<c2[2] else c2[2]]
	return g