import bpy
import math
from ..core import composition

def rampToImage(key, ramp):
    img = bpy.data.images[key]
    w, h = img.size
    print("ramp image size", w, h)
    px = [0.0]*4*w*h
    d = 2**(math.ceil(math.log(ramp.data[-1][0], 2)))

    for i in range(w):
        c = ramp.eval(d*i/w)
        
        for j in range(h):
            idx = j*w + i
            px[4*idx  ] = c.x
            px[4*idx+1] = c.y
            px[4*idx+2] = c.z
            px[4*idx+3] = 1
    
    img.pixels = px

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