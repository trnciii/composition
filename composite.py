import bpy
import numpy as np
import time
import os
import numpy as np

import composition
col = composition.color

def terminate():
    bpy.data.scenes["Scene"].node_tree.nodes["Alpha Over"].inputs[0].default_value = 0
    bpy.data.scenes["Scene"].node_tree.nodes["Alpha Over"].inputs[0].default_value = 1

    for name in dir():
        if not name.startswith('_'):
            print(name)
            del globals()[name]


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


t1 = 'target1'
t2 = 'target2'
t1s = 'target1s'
t2s = 'target2s'
m1 = 'mask1'
m2 = 'mask2'
d1 = 'depth1'
d2 = 'depth2'
nt = 'nt'
rf = 'reference'
tx1 = 'texture1'
tx2 = 'texture2'

cmp = composition.bi.Context()
cmp.bindImage(t1)
cmp.bindImage(t2)
cmp.bindImage(m1)
cmp.bindImage(m2)
cmp.bindImage(d1)
cmp.bindImage(d2)
cmp.bindImage(nt)
cmp.bindImage(rf)
cmp.bindImage(t1s)
cmp.bindImage(t2s)

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
    (0.25, [0.5, 0.4, 0.8]),
    (0.3, [0.5, 0.1, 0.1]),
    (0.65, [0.8, 0.7, 0.2]),
    (1.5, [1, 1, 0.95])
]


def target1():
    ramp = col.RampData(ramp_green2, 'const')    
    # print(ramp)
    # remap = col.basis.ramp(col.basis.sumRadianceRGB, ramp)
    # ramp = 'ColorRamp'
    l =  bpy.data.objects['Sphere.001'].location

    composition.bi.rampToImage(tx1, ramp)

    remap = composition.bi.ramp(col.basis.sumRadianceRGB, 'ColorRamp')
    # remap = col.basis.cel_diffuse(ramp, list(l))

    return remap

def target2():
    # ramp = col.RampData(ramp_red0, 'const')
    # print(ramp)
    # ramp = composition.bi.sliceImage('a.png', 0.5)
    ramp = 'ColorRamp.001'

    composition.bi.rampToImage(tx2, ramp)


    remap = composition.bi.ramp(col.basis.sumRadianceRGB, ramp)
    # remap = col.mix(remap, col.basis.radiance, 0.25)
    # remap = col.mul(remap, col.basis.radiance)
    # remap = col.mul(remap, col.basis.const(6, 6, 6))
    
    return remap


def match(words, query):
    res = True
    for i in range(len(query)):
        if len(query[i])>0:
            res = res and words[i] == query[i]
    return res

def main_cmp():
    cmp.load(nt, cmp.path+"/im_nontarget")

    hits = {}
    files = os.listdir(cmp.path)
    for file in files:
        words = file.split('_')
        query = ['hit', '', '256', 'ex']
        if match(words, query):
            h = composition.core.Hits()
            h.load(cmp.path+'/'+file)
            hits[words[1]] = h

    # remap = [target1(), target2()]
    remap = [col.basis.radiance]*2
    t = [t1, t2]
    for i in range(len(hits)):
        tPrev = time.time()
        cmp.hitsToImage(hits[str(i)], t[i], remap[i])
        print(time.time() - tPrev)

    return

def main_im():

    hits = {}
    files = os.listdir(cmp.path)
    for file in files:
        words = file.split('_')
        query = ['hit', '', '16', 'all']
        if match(words, query):
            h = composition.core.Hits()
            h.load(cmp.path+'/'+file)
            hits[words[1]] = h

    m = [m1, m2]
    d = [d1, d2]
    for i in range(len(hits)):
        cmp.mask(hits[str(i)], m[i], 16)
        cmp.depth(hits[str(i)], d[i], 16)

    return


main_cmp()
# main_im()
terminate()
