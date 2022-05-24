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
		self._data = [(f, np.array(s)) for f, s in sorted(pairs)]


	@property
	def eval(self):
		return self.evalTypes[self._mode]
		

	def evalConst(self, u):
		c = self._data[0][1]
		for first, second in self._data:
			if u>first: c = second
			else: return c
		return c


	def evalLinear(self, u):
		if u<=self._data[0][0]:
			return self._data[0][1]

		if u>=self._data[-1][0]:
			return self._data[-1][1]
		

		x1, y1 = self._data[0]

		for x2, y2 in self._data[1:]:
			if x1<=u and u<x2:
				return ((x2-u)*y1 + (u-x1)*y2)/(x2-x1)
			x1, y1 = x2, y2

		return np.zeros(3)


	def __str__(self):
		s = "ramp <" + str(self._mode) + ">\n"
		for e in self._data:
			s = s + "  " + str(e) + "\n"
		
		return s