import multiprocessing as mp
from concurrent.futures import ThreadPoolExecutor

from . import composition as cmp

def replace(res, i, hits, color):
    hit = hits[i]
    c = color(hit)
    res[i] = [hit.weight.x*c.x, hit.weight.y*c.y, hit.weight.z*c.z]

def hitsToImage(hits, image, color):
    print("using python for replacement")

    im  = [[0.]*3 for i in range(len(image.pixels))]
    res = [[0.]*3 for i in range(len(hits.data))]

    for hit in hits.data:
        c = color(hit)
        im[hit.pixel][0] += hit.weight.x * c.x
        im[hit.pixel][1] += hit.weight.y * c.y
        im[hit.pixel][2] += hit.weight.z * c.z

    # for i in range(len(hits.data)):
    #     replace(res, i, hits.data, color)
    # for i in range(len(hits.data)):
    #     im[hits.data[i].pixel][0] += res[i][0]
    #     im[hits.data[i].pixel][1] += res[i][1]
    #     im[hits.data[i].pixel][2] += res[i][2]

    for i in range(len(image.pixels)):
        image.pixels[i] = cmp.vec3(im[i][0], im[i][1], im[i][2])

def mask(hits, image, nRay):
    count = [0]*len(image.pixels)

    for hit in hits.data:
        count[hit.pixel] += 1

    for i in range(len(image.pixels)):
        image.pixels[i] = cmp.vec3(count[i]/nRay, 0, 0)

def depth(hits, image, nRay):
    d = [0]*len(image.pixels)
    count = [0]*len(image.pixels)
    max = 0

    for hit in hits.data:
        d[hit.pixel] += hit.depth
        count[hit.pixel] += 1
        max = hit.depth if hit.depth > max else max
    
    if max>0:
        for i in range(len(image.pixels)):
            if count[i] > 0:
                image.pixels[i] = cmp.vec3(d[i]/nRay/max, 0, 0)

    return max
    