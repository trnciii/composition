import bpy
import numpy as np
import time
import os
import numpy as np

import composition
col = composition.color


targetMaterials = ['target1', 'target2']
nt = 'nt'


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


def target0():
    ramp = col.RampData(ramp_green2, 'const')    
    # print(ramp)
    # remap = col.basis.ramp(col.basis.sumRadianceRGB, ramp)
    # ramp = 'ColorRamp'
    # l =  bpy.data.objects['Sphere.001'].location

    composition.bi.rampToImage(targetMaterials[0]+'_texture', ramp)

    remap = composition.bi.ramp(col.basis.sumRadianceRGB, ramp)
    # remap = col.basis.cel_diffuse(ramp, list(l))

    return remap

def target1():
    # ramp = col.RampData(ramp_red0, 'const')
    # print(ramp)
    # ramp = composition.bi.sliceImage('a.png', 0.5)
    ramp = 'ColorRamp.001'
    composition.bi.rampToImage(targetMaterials[1]+'_texture', ramp)

    remap = composition.bi.ramp(col.basis.sumRadianceRGB, ramp)
    # remap = col.mix(remap, col.basis.radiance, 0.25)
    # remap = col.mul(remap, col.basis.radiance)
    # remap = col.mul(remap, col.basis.const(6, 6, 6))
    
    return remap


def main_cmp(cmp):
    print('\033[1mconversion\033[0m')
    
    cmp.load(nt, cmp.path+"/im_nontarget")
    print()

    hits = cmp.readHits(['hit', '', '16', 'ex'])
    remap = [target0(), target1()]
#    remap = [col.basis.radiance]*len(hits)
    
    for i in range(len(hits)):
        print('converting\033[33m', targetMaterials[i], '\033[0m', end='')
        tPrev = time.time()
        cmp.hitsToImage(hits[targetMaterials[i]], targetMaterials[i], remap[i])
        print(time.time() - tPrev, '\n')

    return

def main_im(cmp):
    print('\033[1mmasking\033[0m')

    hits = cmp.readHits(['hit', '', '16', 'all'])
    m = [t+'_mask' for t in targetMaterials]
    d = [t+'_depth' for t in targetMaterials]
    
    for i in range(len(hits)):
        cmp.mask(hits[targetMaterials[i]], m[i], 16)
        cmp.depth(hits[targetMaterials[i]], d[i], 16)

    print()
    return

def main():
    cmp = composition.bi.Context()

    cmp.bindImage(nt)
    cmp.addImages(targetMaterials)
    cmp.addImages([t+'_mask' for t in targetMaterials])
    cmp.addImages([t+'_depth' for t in targetMaterials])
    cmp.addImages([t+'_texture' for t in targetMaterials], 256, 16)

    cmp.listFiles()

    main_cmp(cmp)
    main_im(cmp)
    return

print('\033[36mcomposite.py\033[0m')
main()

# delete variables
for name in dir():
    if not name.startswith('_'):
        # print(name)
        del globals()[name]