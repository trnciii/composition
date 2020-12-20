import numpy as np

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

class Ramp():
	def __init__(self, pairs, mode='linear'):
		self.mode = mode
		
		pairs.sort()
		self.pairs = []
		for pair in pairs:
			self.pairs.append((pair[0], np.array(pair[1])))
		
	def print(self):
		print("ramp")
		for e in self.pairs:
			print("--", e)
		print()
		
	def evalConst(self, u):
		c = self.pairs[0][1]
		for pair in self.pairs:
			if u>pair[0]: c = pair[1]
		return c

	def evalLinear(self, u):
		if u<=self.pairs[ 0][0]: return self.pairs[ 0][1]
		if u>=self.pairs[-1][0]: return self.pairs[-1][1]
		
		x1 = self.pairs[0][0]
		y1 = self.pairs[0][1]
		
		for pair in self.pairs[1:]:
			x2 = pair[0]
			y2 = pair[1]
			
			if x1<=u and u<x2:
				return ((x2-u)*y1 + (u-x1)*y2)/(x2-x1)

			x1 = x2
			y1 = y2

		return [0,0,0]
		
	def evaluator(self):
		if self.mode is 'const': return self.evalConst
		if self.mode is 'linear': return self.evalLinear