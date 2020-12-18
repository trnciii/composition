from . import composition as cmp

def hitsToImage(hits, renderpass, id, color):
    im = [0]*(3*renderpass.width*renderpass.height)
    for i in range(hits.size()):
        hit = hits.element(i)
        c = color(hit)
        im[3*hit.pixel  ] += hit.weight.x * c[0]
        im[3*hit.pixel+1] += hit.weight.y * c[1]
        im[3*hit.pixel+2] += hit.weight.z * c[2]
    for i in range(renderpass.width*renderpass.height):
        renderpass.set(id, i, im[3*i], im[3*i+1], im[3*i+2])

def mask(hits, renderpass, id, nRay):
	length = renderpass.width*renderpass.height
	count = [0]*3*length

	for i in range(hits.size()):
		hit = hits.element(i)
		count[hit.pixel] += 1

	for i in range(length):
		renderpass.set(id, i, count[i]/nRay, 0, 0)

def depth(hits, renderpass, id, nRay):
	length = renderpass.width*renderpass.height
	d = [0]*3*length
	count = [0]*3*length
	max = 0

	for i in range(hits.size()):
		hit = hits.element(i)
		d[hit.pixel] += hit.depth
		count[hit.pixel] += 1
		max = hit.depth if hit.depth > max else max
	
	if max>0:
		for i in range(length):
			if count[i] > 0:
				renderpass.set(id, i, d[i]/nRay/max, 0, 0)

	return max
	