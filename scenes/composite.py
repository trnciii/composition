import bpy
import numpy as np
import time
import os
import numpy as np

import composition
col = composition.color

def validatgeGlobals():
    req = ['targetMaterials', 'targetRemap', 'param']

    for r in req:
        if not r in globals():
            print('Could not find', r)
            return False

    if not len(targetMaterials) is len(targetRemap):
        print('number of target material and remap does not agree')
        return False

    return True

def main_cmp(cmp):
    print('\033[1mconversion\033[0m')
    
    cmp.load('nt', cmp.path+"im_nontarget")
    cmp.load('pt', cmp.path+'im_pt')
    print()

    hits = cmp.readHits(['hit', '', str(param.nRay), 'ex'])    
    for i in range(len(hits)):
        print('converting\033[33m', targetMaterials[i], '\033[0m', end='')
        tPrev = time.time()
        cmp.hitsToImage(hits[targetMaterials[i]], targetMaterials[i], targetRemap[i])
        print(time.time() - tPrev, '\n')

    return

def main_im(cmp):
    print('\033[1mmasking\033[0m')

    hits = cmp.readHits(['hit', '', str(param.nRay), 'all'])
    m = [t+'_mask' for t in targetMaterials]
    d = [t+'_depth' for t in targetMaterials]
    
    for i in range(len(hits)):
        cmp.mask(hits[targetMaterials[i]], m[i], 16)
        cmp.depth(hits[targetMaterials[i]], d[i], 16)

    print()
    return

def composite():
    cmp = composition.bi.Context()

    cmp.addImages(['nt', 'pt'])
    cmp.addImages(targetMaterials)
    cmp.addImages([t+'_mask' for t in targetMaterials])
    cmp.addImages([t+'_depth' for t in targetMaterials])

    cmp.listFiles()

    main_cmp(cmp)
    main_im(cmp)
    return

print('\033[36mcomposite.py\033[0m')
if validatgeGlobals():
    composite()
print('---- end of composite.py ----\n')