import bpy
import time
import composition
col = composition.color
HitsType = composition.HitsType

param = composition.core.PPMParam()
param.nRay = 16
param.nPhoton = 100000
param.itr = 200


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


def cel0():
    l =  bpy.data.objects['Sphere.001'].location
    ramp = col.RampData(ramp_green2, 'const')    
    return col.basis.cel_specular(ramp, list(l))

def cel1():
    l =  bpy.data.objects['Sphere.001'].location
    ramp = col.RampData(ramp_red0, 'linear')    
    return col.basis.cel_specular(ramp, list(l))


def target0():
    # print(ramp)
    # remap = col.basis.ramp(col.basis.sumRadianceRGB, ramp)
    ramp = 'ColorRamp'

    composition.rampToImage(targetMaterials[0]+'_texture', ramp, 256, 16)

    remap = composition.ramp(col.basis.sumRadianceRGB, ramp)

    return remap

def target1():
    # ramp = col.RampData(ramp_red0, 'const')
    # print(ramp)
    # ramp = composition.sliceImage('a.png', 0.5)
    ramp = 'ColorRamp.001'
    composition.rampToImage(targetMaterials[1]+'_texture', ramp, 256, 16)

    remap = composition.ramp(col.basis.sumRadianceRGB, ramp)
    # remap = col.mix(remap, col.basis.radiance, 0.25)
    # remap = col.mul(remap, col.basis.radiance)
    # remap = col.mul(remap, col.basis.const(6, 6, 6))
    
    return remap

spheres = ['Sphere.001', 'Sphere.002', 'Sphere']
meshes = ['floor', 'left', 'back', 'right']
targetMaterials = ['target1', 'target2']

targetRemap = {
    'target1' : target0(), 
    'target2' : target1()
}


def render():
    print('\033[36mrendering\033[0m')
    cmp = composition.Context()
    cmp.scene.create(spheres, meshes, targetMaterials)
    cmp.setTargets(targetMaterials)
    print(cmp.scene.data)
    time.sleep(0.1)

    cmp.pt_ref('pt', 1000)
    cmp.saveImage('pt')

    cmp.pt_nt('nt', 1000)
    cmp.saveImage('nt')

    for t in cmp.targetNames:
        cmp.genHits_ex(t, param.nRay, param.R0)
        cmp.genHits_all(t, param.nRay, param.R0)

    print()

    for k, h in cmp.hits.items():
        print(k)

        if k.type is composition.HitsType.EX:
            # cmp.radiance_pt(h, 10000)
            cmp.radiance_ppm(h, param)
        
        cmp.saveHits(k, param.nRay)

        print()


    cmp.remapAll({n : col.basis.radiance for n in cmp.targetNames})

    print()
    return

def remap():
    print('\033[36mremapping\033[0m')

    cmp = composition.Context()
    cmp.setTargets(targetMaterials)
    cmp.loadAll(param.nRay)

    for i in cmp.hits.keys():
        print(i)

    cmp.remapAll(targetRemap)
    cmp.maskAll()

    print()
    return

def nprr():
    cmp = composition.bi.Context()
    cmp.scene.create(spheres, meshes, targetMaterials)
    cmp.setTargets(targetMaterials)
    print(cmp.scene.data)
    time.sleep(0.1)

    nodes = {
        'target1' : 'ColorRamp',
        'target2' : 'ColorRamp.001'
    }
    
    remap = []

    # the order of remapping may not match the materials
    for key, node in nodes.items():
        image = key+'_texture'
        composition.bi.rampToImage(image, node, 256, 16)
        remap.append((composition.sliceImage(image, 8), 0, 25))

    cmp.nprr('nprr', 1000, remap)

    print('-- end nprr --')

nprr()
# render()
# remap()