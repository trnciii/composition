from . import composition as cmp

def hitsToImage(hits, renderpass, id, color):
    im = [0]*(3*renderpass.length)
    for hit in hits.data:
        c = color(hit)
        im[3*hit.pixel  ] += hit.weight.x * c[0]
        im[3*hit.pixel+1] += hit.weight.y * c[1]
        im[3*hit.pixel+2] += hit.weight.z * c[2]
    for i in range(renderpass.length):
        renderpass.set(id, i, im[3*i], im[3*i+1], im[3*i+2])

def mask(hits, renderpass, id, nRay):
	count = [0]*3*renderpass.length

	for hit in hits.data:
		count[hit.pixel] += 1

	for i in range(renderpass.length):
		renderpass.set(id, i, count[i]/nRay, 0, 0)

def depth(hits, renderpass, id, nRay):
	d = [0]*3*renderpass.length
	count = [0]*3*renderpass.length
	max = 0

	for hit in hits.data:
		d[hit.pixel] += hit.depth
		count[hit.pixel] += 1
		max = hit.depth if hit.depth > max else max
	
	if max>0:
		for i in range(renderpass.length):
			if count[i] > 0:
				renderpass.set(id, i, d[i]/nRay/max, 0, 0)

	return max
	