import bpy
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
tx1 = 'texture1'
tx2 = 'texture2'

hits1 = composition.core.Hits()
hits2 = composition.core.Hits()

#hits1.load(path + "hits1_selected_256")
#hits2.load(path + "hits2_selected_256")

hits1.load(path + "hits1_256")
hits2.load(path + "hits2_256")

cmp = composition.Context()
cmp.bindImage(t1)
cmp.bindImage(t2)
cmp.bindImage(m1)
cmp.bindImage(m2)
cmp.bindImage(d1)
cmp.bindImage(d2)
cmp.bindImage(nt)

def terminate():
    bpy.data.scenes["Scene"].node_tree.nodes["Alpha Over"].inputs[0].default_value = 0
    bpy.data.scenes["Scene"].node_tree.nodes["Alpha Over"].inputs[0].default_value = 1

    for name in dir():
        if not name.startswith('_'):
            print(name)
            del globals()[name]
    
    print("---- end ----")
    
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
    
def setAlpha(cmp, key_color, key_alpha, key_out):
    ps = cmp.renderpass
    im = composition.core.getImage(ps, cmp.bind[key_color])
    a = composition.core.getImage(ps, cmp.bind[key_alpha])
    for i in range(ps.length):
        im[4*i+3] = a[4*i]
    bpy.data.images[key_out].pixels = im


def masks():
    print("mask")
    cmp.mask(hits1, m1, 64)
    cmp.mask(hits2, m2, 64)

    print("depth")
    cmp.depth(hits1, d1, 64)
    cmp.depth(hits2, d2, 64)


# define consts
const_orange = col.basis.const(0.8, 0.3, 0.1)
const_green = col.basis.const(0.2, 0.9, 0.4)

# define ramps
ramp_green0 = [
    (0.07, [0.03, 0.1, 0.03]),
    (0.3, [0.1, 0.5, 0.1 ]),
    (0.5, [0.6, 0.8, 0.2]),
    (0.8, [0.6, 0.8, 0.2]),
    (1, [0.9, 1, 0.9])
]

ramp_green1 = [
    (0.07, [0.03, 0.1, 0.03]),
    (0.5, [0.1, 0.5, 0.1 ]),
    (0.8, [0.6, 0.8, 0.2]),
    (1, [0.9, 1, 0.9])
]

ramp_green2 = [
    (0, [0.03, 0.1, 0.03]),
    (0.07, [0.1, 0.5, 0.1 ]),
    (0.3 , [0.6, 0.8, 0.2]),
    (0.8 , [0.9, 1, 0.9]),
]

ramp_red0 = [
    (0, [0.1, 0.02, 0.02]),
    (0.3, [0.5, 0.1, 0.1]),
    (0.65, [0.8, 0.7, 0.2]),
    (1.5, [1, 1, 0.95])
]


def target1():
    ramp = col.Ramp(ramp_green2, 'const')

    ramp.print()
    composition.rampToImage(tx1, ramp)

    remap = col.basis.ramp(col.basis.sumRadianceRGB, ramp.eval)
#    remap = col.mul(remap, col.basis.radiance)
    remap = col.mix(remap, col.basis.radiance, 0.8)

    cmp.hitsToImage(hits1, t1, remap)
    
def target2():
    ramp = col.Ramp(ramp_red0, 'linear')

    ramp.print()
    composition.rampToImage(tx2, ramp)

    remap = col.basis.ramp(col.basis.sumRadianceRGB, ramp.eval)
    remap = col.mix(remap, col.basis.radiance, 0.4)
    
    cmp.hitsToImage(hits2, t2, remap)


#cmp.load(nt, path+"nontarget")
#masks()

print("converting hits to color")
time0 = time.time()

target1()
#target2()

print("time:", time.time()-time0)

terminate()