from . import composition

class PPMParam:
	def __init__(self):
		self.nRay = 8
		self.nPhoton = 10000
		self.itr = 10
		self.alpha = 0.6
		self.R0 = 1.0

	def __str__(self):
		return\
			"nRay    " + str(self.nRay) + "\n"\
			"nPhoton " + str(self.nPhoton) + "\n"\
			"itr     " + str(self.itr) + "\n"\
			"alpha   " + str(self.alpha) + "\n"\
			"R0      " + str(self.R0) + "\n"

def hitsToImage_py(hits, image, color):
    print("using python for replacement")

    for hit in hits:
        image.pixels[hit.pixel] += hit.weight*color(hit)
