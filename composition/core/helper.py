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

    im  = [[0.]*3 for i in range(len(image.pixels))]
    res = [[0.]*3 for i in range(len(hits))]

    for hit in hits:
        c = color(hit)
        im[hit.pixel][0] += hit.weight.x * c.x
        im[hit.pixel][1] += hit.weight.y * c.y
        im[hit.pixel][2] += hit.weight.z * c.z

    for i in range(len(image.pixels)):
        image.pixels[i] = composition.vec3(im[i][0], im[i][1], im[i][2])
