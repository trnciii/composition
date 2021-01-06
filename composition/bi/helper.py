import bpy
import math

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