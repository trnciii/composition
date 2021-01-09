from . import composition

class PPMParam:
	def __init__(self):
		self.nRay = 8
		self.nPhoton = 10000
		self.itr = 10
		self.alpha = 0.6
		self.R0 = 1.0

	def __repr__(self):
		return\
			"nRay    " + str(self.nRay) + "\n"\
			"nPhoton " + str(self.nPhoton) + "\n"\
			"itr     " + str(self.itr) + "\n"\
			"alpha   " + str(self.alpha) + "\n"\
			"R0      " + str(self.R0)

def ppm(renderpass, id, param, scene):
	composition.ppm(renderpass, id, param.nRay, param.nPhoton, param.itr, param.alpha, param.R0, scene)