import numpy as np

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