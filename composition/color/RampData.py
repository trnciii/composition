import numpy as np

class RampData():

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
		if mode in self.evalTypes:
			self._mode = mode
		else:
			print("invalid mode")

	@property
	def data(self):
		return self._data

	@data.setter
	def data(self, pairs):
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
		return [c[0], c[1], c[2]]

	def evalLinear(self, u):
		if u<=self._data[ 0][0]:
			c = self._data[0][1]
			return [c[0], c[1], c[2]]
		if u>=self._data[-1][0]:
			c = self._data[-1][1]
			return [c[0], c[1], c[2]]
		
		x1 = self._data[0][0]
		y1 = self._data[0][1]
		
		for pair in self._data[1:]:
			x2 = pair[0]
			y2 = pair[1]
			
			if x1<=u and u<x2:
				c = ((x2-u)*y1 + (u-x1)*y2)/(x2-x1)
				return [c[0], c[1], c[2]]

			x1 = x2
			y1 = y2

		return [0, 0, 0]

	def __str__(self):
		s = "ramp <" + str(self._mode) + ">\n"
		for e in self._data:
			s = s + "  " + str(e) + "\n"
		
		return s