import bpy
import math
import numpy as np

from .. import color

def rampToImage(name, ramp, w, h):
	img = addImage(name, w, h)
	nodes = bpy.context.scene.node_tree.nodes

	if isinstance(ramp, np.ndarray):
		end = ramp.shape[0]-1
		ev = lambda x: ramp[int(x)]

	elif ramp in nodes.keys():
		end = 1
		ev = nodes[ramp].color_ramp.evaluate

	elif isinstance(ramp, color.RampData):
		end = 2**(math.ceil(math.log(ramp.data[-1][0], 2)))
		ev = ramp.eval

	else:
		end = 1
		ev = lambda x: [1,0,1,1]


	sampled = np.array([ev(x) for x in np.linspace(0, end, w)])

	if sampled.shape[1] == 3:
		sampled = np.insert(sampled, 3, 1, axis=1)

	img.pixels = np.tile(sampled.flatten(), h)


def sliceImage(key, y):
	image = bpy.data.images[key]
	w, h = image.size
	pixels = np.array(image.pixels).reshape((h, w, 4))

	y = max(0, min(h-1, int(y*h)))
	return pixels[y, :, :3]


def ramp(coord, name):
	nodes = bpy.context.scene.node_tree.nodes

	if name in nodes.keys():
		ev = nodes[name].color_ramp.evaluate

		def f(hit):
			return ev(coord(hit))
			# return [rs[0], rs[1], rs[2]]

		return f

	else:
		return color.basis.ramp(coord, name)

def addImage(name, w, h):
	if name not in bpy.data.images.keys():
		print('add image \''+name+'\'', w, h)
		bpy.data.images.new(name, w, h)
	else:
		bpy.data.images[name].scale(w,h)

	return bpy.data.images[name]