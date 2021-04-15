import bpy
import math
from ..core.composition import vec3
from .. import color

def rampToImage(name, ramp, w, h):	
	def case_RampData():
		d = 2**(math.ceil(math.log(ramp.data[-1][0], 2)))
		c = ramp.eval(d*i/w)
		return [c.x, c.y, c.z, 1]
	
	def case_image():
		d = len(ramp)
		return ramp[max(0, min(w-1, int(d*i/w)))] + [1]

	img = addImage(name, w, h)
	px = [[0.0]*4 for i in range(w*h)]
	nodes = bpy.context.scene.node_tree.nodes
	
	for i in range(w):
		c = [1., 0., 1.]
		if isinstance(ramp, color.RampData):
			c = case_RampData()
		elif isinstance(ramp, list):
			c = case_image()
		elif ramp in nodes.keys():
			c = list(nodes[ramp].color_ramp.evaluate(i/w))
		else:
			print('failed to find ramp data <{}>'.format(name))
			return

		for j in range(h):
			idx = j*w + i
			px[idx] = c
			 
	img.pixels = sum(px, [])


def sliceImage(key, y):
	im = bpy.data.images[key]
	w, h = im.size
	
	p = [[0.]*3 for i in range(w)]
	y = max(0, min(h-1, int(y*h)))
	for x in range(w):
		i = y*w+x
		p[x][0] = im.pixels[4*i  ]
		p[x][1] = im.pixels[4*i+1]
		p[x][2] = im.pixels[4*i+2]
	return p

def ramp(coord, name):
	nodes = bpy.context.scene.node_tree.nodes

	if name in nodes.keys():
		ev = nodes[name].color_ramp.evaluate
	
		def f(hit):
			rs = ev(coord(hit))
			return vec3(rs[0], rs[1], rs[2])

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