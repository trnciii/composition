import bpy
import time
import numpy as np

import composition
col = composition.color

# define ramps
ramp_green = [
    (0, [0.03, 0.1, 0.03]),
    (0.07, [0.1, 0.5, 0.1 ]),
    (0.3 , [0.6, 0.8, 0.2]),
    (0.8 , [0.9, 1, 0.9]),
]

ramp_red = [
    (0, [0.2, 0.1, 0.1]),
    (0.25, [0.5, 0.8, 0.4]),
    (0.3, [0.5, 0.1, 0.1]),
    (0.65, [0.8, 0.7, 0.2]),
    (1.5, [1, 1, 0.95])
]

param_preview = composition.core.PPMParam()
param_preview.nRay = 16
param_preview.nPhoton = 1000
param_preview.itr = 10

param_final = composition.core.PPMParam()
param_final.nRay = 256
param_final.nPhoton = 100000
param_final.itr = 10000

param = param_preview

def target0():
    ramp = col.RampData(ramp_green, 'const')
    # print(ramp)
    # remap = col.basis.ramp(col.basis.sumRadianceRGB, ramp)
    # ramp = 'ColorRamp'
    # l =  bpy.data.objects['Sphere.001'].location

    composition.bi.rampToImage(targetMaterials[0]+'_texture', ramp, 256, 16)

    remap = composition.bi.ramp(col.basis.sumRadianceRGB, ramp)
    # remap = col.basis.cel_diffuse(ramp, list(l))

    return remap

def target1():
    ramp = col.RampData(ramp_red, 'const')
    # print(ramp)
    # ramp = composition.bi.sliceImage('a.png', 0.5)
#    ramp = 'ColorRamp.001'

    def u(hit):
        r = col.basis.sumRadianceRGB(hit)
        r = r**0.5
        r = r*0.7
        return r
    
    composition.bi.rampToImage(targetMaterials[1]+'_texture', ramp, 256, 16)

    remap = composition.bi.ramp(u, ramp)
    remap = col.mul(remap, col.basis.radiance)
    remap = col.mul(remap, col.basis.const(6, 6, 6))
#    remap = col.mix(remap, col.basis.radiance, 0.3)
    
    return remap

def rmFloor():
    ramp = 'ColorRamp.002'
    remap = composition.bi.ramp(col.basis.sumRadianceRGB, ramp)
    return remap

targetMaterials = ['target1', 'target2', 'glossy']
targetRemap = {
    'target1' : target0(),
    'target2' : target1(),
    'glossy' : rmFloor()
}

#targetRemap[1] = col.basis.radiance

spheres = ['Sphere.001', 'Icosphere', 'Sphere']
meshes = ['Sphere.002', 'floor']

def render():
    print('\033[36mrendering\033[0m')

    cmp = composition.bi.Context()
    cmp.scene.create(spheres,meshes,targetMaterials)
    cmp.setTargets(targetMaterials)
    print(cmp.scene.data)

    time.sleep(0.1)

    cmp.pt_ref('pt', 1000)
    cmp.saveImage('pt')

    cmp.ppm_ref('ppm', param)

    cmp.pt_nt('nt', 100)
    cmp.saveImage('nt')
    print()

    for t in cmp.targetNames:
        cmp.genHits_ex(t, param.nRay, param.R0)
        cmp.genHits_all(t, param.nRay, param.R0)

    print()

    for k, h in cmp.hits.items():
        print(k)

        if k.type is composition.HitsType.EX:
            # cmp.radiance_pt(h, 1000)
            cmp.radiance_ppm(h, param)
        
        cmp.saveHits(k, param.nRay)

        print()


    cmp.remapAll({n : col.basis.radiance for n in cmp.targetNames})

def remap():
    print('\033[36mremapping\033[0m')

    cmp = composition.bi.Context()
    cmp.setTargets(targetMaterials)
    cmp.loadAll(param.nRay)

    cmp.remapAll(targetRemap)
    cmp.maskAll()

def nprr():
    cmp = composition.bi.Context()
    cmp.scene.create(spheres, meshes, targetMaterials)
    cmp.setTargets(targetMaterials)
    print(cmp.scene.data)
    time.sleep(0.1)

    nodes = {
        'target1' : col.RampData(ramp_green),
        'target2' : col.RampData(ramp_red),
        'glossy' : 'ColorRamp.002'
    }

    remap = []

    # the order of remapping may not match
    for key, node in nodes.items():
        image = key + '_texture'
        composition.bi.rampToImage(image, node, 256, 16)
        array = np.array(composition.sliceImage(image, 8))
        remap.append((array.flatten(), 0, 35))

    cmp.nprr('nprr', 1000, remap)

    print('-- end nprr --')

nprr()
render()
remap()