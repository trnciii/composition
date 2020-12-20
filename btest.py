import bpy
import functools
import numpy as np
import time

import composition
col = composition.color

path = bpy.path.abspath("//result\\")
t1 = 'target1'
t2 = 'target2'
t1l = 'target1_linear'
t2l = 'target2_linear'
m1 = 'mask1'
m2 = 'mask2'
d1 = 'depth1'
d2 = 'depth2'
nt = 'nt'
rf = 'reference'
tx = 'texture'

def allHitsToImage(cmp, h1, h2, k1, k2, c):
    cmp.hitsToImage(h1, k1, c)
    cmp.hitsToImage(h2, k2, c)
    
def ppm(cmp, h1, h2):
    R0 = 0.5
    itr = 1000
    nPhoton = 10000
    alpha = 0.6
    cmp.ppm(cmp.target1, h1, R0, itr, nPhoton, alpha)
    cmp.ppm(cmp.target2, h2, R0, itr, nPhoton, alpha)

def purity(rad, ch):
    a = (rad[0] + rad[1] + rad[2])/3
    p = rad[ch]-a
    if 0<p:
        return p*10
    else:
        return 0

def cel(hit):
    l = [-hit.p.x, -hit.p.y, 6-hit.p.z]
    dot = l[0]*hit.n.x + l[1]*hit.n.y + l[2]*hit.n.z
    dot = 0.5*dot + 0.5
    rad = col.radiance(hit)
    m = min(rad)
    M = max(rad)
    g = purity(rad, 1)      
#    return [g, g, g]
    c = [0.3, 0.1, 0.03] if dot<0 else [0.8, 0.6, 0.1]
    return [c[0], (1-g)*c[1]+g, c[2]]
    
def setAlpha(cmp, key_color, key_alpha, key_out):
    ps = cmp.renderpass
    im = composition.core.getImage(ps, cmp.bind[key_color])
    a = composition.core.getImage(ps, cmp.bind[key_alpha])
    for i in range(ps.width*ps.height):
        im[4*i+3] = a[4*i]
#    bpy.data.images[key_out].pixels = im
    bpy.data.images[key_out].pixels = im

def sumRadianceRGB(hit):
    tau = col.radiance(hit)
    return (tau[0] + tau[1] + tau[2])

def hitToRamp(coord, ramp):
    def f(hit):
        return ramp(coord(hit))
    return f
    
def rampToImage(key, ramp):
    img = bpy.data.images[key]
    w, h = img.size
    print("ramp image size", w, h)
    px = [0.0]*4*w*h
    
    for i in range(w):
        c = ramp.evaluator()(i/w)
        
        for j in range(h):
            idx = j*w + i
            px[4*idx  ] = c[0]
            px[4*idx+1] = c[1]
            px[4*idx+2] = c[2]
            px[4*idx+3] = 1
    
    img.pixels = px



#hits1_ex = composition.core.Hits()
#hits2_ex = composition.core.Hits()
hits1_total = composition.core.Hits()
hits2_total = composition.core.Hits()

#hits1_ex.load(path + "hits1")
#hits2_ex.load(path + "hits2")
hits1_total.load(path + "hits1_total")
hits2_total.load(path + "hits2_total")
print()

cmp = composition.Context()
cmp.bindImage(t1)
cmp.bindImage(t2)
cmp.bindImage(m1)
cmp.bindImage(m2)
cmp.bindImage(d1)
cmp.bindImage(d2)
cmp.bindImage(nt)


#cmp.load(nt, path+"nontarget")

#print("mask")
#cmp.mask(hits1_total, m1, 64)
#cmp.mask(hits2_total, m2, 64)

#print("depth")
#cmp.depth(hits1_total, d1, 64)
#cmp.depth(hits2_total, d2, 64)

# define consts
const_orange = col.const(0.8, 0.3, 0.1)
const_green = col.const(0.2, 0.9, 0.4)

# define ramps
ramp_green0 = [(0.07, [0.03, 0.1, 0.03]),
    (0.3, [0.1, 0.5, 0.1 ]),
    (0.5, [0.6, 0.8, 0.2]),
    (0.8, [0.6, 0.8, 0.2]),
    (1, [0.9, 1, 0.9])]
    
ramp_green1 = [(0.07, [0.03, 0.1, 0.03]),
    (0.3, [0.1, 0.5, 0.1 ]),
    (0.8, [0.6, 0.8, 0.2]),
    (1, [0.9, 1, 0.9])]
    
ramp_green2 = [(0, [0.03, 0.1, 0.03]),
    (0.07, [0.1, 0.5, 0.1 ]),
    (0.3 , [0.6, 0.8, 0.2]),
    (0.8 , [0.9, 1, 0.9])]

ramp_red0 = [(0, [0.1, 0.02, 0.02]),
    (0.3, [0.5, 0.1, 0.1]),
    (0.65, [0.8, 0.6, 0.6]),
    (1.5, [1, 1, 0.95])]

# create a ramp
ramp = col.Ramp(ramp_green2, 'linear')
ramp.print()
    
rampToImage(tx, ramp)

remap = hitToRamp(sumRadianceRGB, ramp.evaluator())
remap = col.mul(remap, col.radiance)


print("converting hits to color")
t0 = time.time()

cmp.hitsToImage(hits1_total, t1, remap)

#ramp.mode = 'const'
#remap = hitToRamp(sumRadianceRGB, ramp.evaluator())
#cmp.hitsToImage(hits2_total, t2, remap)

print("time:", time.time()-t0)


# update scene and delete variables
bpy.data.scenes["Scene"].node_tree.nodes["Alpha Over"].inputs[0].default_value = 0
bpy.data.scenes["Scene"].node_tree.nodes["Alpha Over"].inputs[0].default_value = 1

for name in dir():
    if not name.startswith('_'):
#        print(name)
        del globals()[name]

print("---- end ----")