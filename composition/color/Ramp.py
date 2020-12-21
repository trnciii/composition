import numpy as np

class Ramp():

	def __init__(self, pairs, mode='linear'):
		
		self.evalTypes = {
			'const': self.evalConst, 
			'linear': self.evalLinear
		}

		self.mode = mode
		self.data = pairs

	@property
	def mode(self):
		return self._mode

	@mode.setter
	def mode(self, mode):
		print("in mode setter")
		if mode in self.evalTypes:
			self._mode = mode
		else:
			print("invalid mode")

	@property
	def data(self):
		return self._data

	@data.setter
	def data(self, pairs):
		print("in data setter")
		pairs.sort()
		self._data = []
		for pair in pairs:
			self._data.append((pair[0], np.array(pair[1])))

	@property
	def eval(self):
		return self.evalTypes[self._mode]
		
	def evalConst(self, u):
		c = self._data[0][1]
		for pair in self._data:
			if u>pair[0]: c = pair[1]
		return c

	def evalLinear(self, u):
		if u<=self._data[ 0][0]: return self._data[ 0][1]
		if u>=self._data[-1][0]: return self._data[-1][1]
		
		x1 = self._data[0][0]
		y1 = self._data[0][1]
		
		for pair in self._data[1:]:
			x2 = pair[0]
			y2 = pair[1]
			
			if x1<=u and u<x2:
				return ((x2-u)*y1 + (u-x1)*y2)/(x2-x1)

			x1 = x2
			y1 = y2

		return [0,0,0]

	def print(self):
		print("ramp:", "mode =", self._mode)
		for e in self._data:
			print("--", e)
		print()