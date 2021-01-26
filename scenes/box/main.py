import bpy
import time
import composition
col = composition.color

param = composition.core.PPMParam()
param.nRay = 16
param.nPhoton = 1000
param.itr = 200


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

    composition.bi.rampToImage(targetMaterials[0]+'_texture', ramp, 256, 16)

    remap = composition.bi.ramp(col.basis.sumRadianceRGB, ramp)
    # remap = col.basis.cel_diffuse(ramp, list(l))

    return remap

def target1():
    # ramp = col.RampData(ramp_red0, 'const')
    # print(ramp)
    # ramp = composition.bi.sliceImage('a.png', 0.5)
    ramp = 'ColorRamp.001'
    composition.bi.rampToImage(targetMaterials[1]+'_texture', ramp, 256, 16)

    remap = composition.bi.ramp(col.basis.sumRadianceRGB, ramp)
    # remap = col.mix(remap, col.basis.radiance, 0.25)
    # remap = col.mul(remap, col.basis.radiance)
    # remap = col.mul(remap, col.basis.const(6, 6, 6))
    
    return remap

spheres = ['Sphere.001', 'Sphere.002', 'Sphere']
meshes = ['floor', 'left', 'back', 'right']
targetMaterials = ['target1', 'target2']

targetRemap = [target0(), target1()]


def render():
    print('\033[36mrendering\033[0m')
    cmp = composition.bi.Context()

    cmp.addImages(['nt', 'pt'])

    cmp.scene.addSpheres(spheres)
    cmp.scene.addMeshes(meshes)
    cmp.scene.setCamera()

    cmp.setTargets(targetMaterials)
    for t in cmp.targets:
        cmp.scene.addTarget(t)

    cmp.scene.print()
    time.sleep(0.1)

    cmp.pt_ref('pt', 1000)
    cmp.save('pt', cmp.path+'im_pt')

    cmp.pt_nt('nt', 1000)
    cmp.save('nt', cmp.path+"im_nontarget")
    print('')

    cmp.ppm_targets(param)
    cmp.ppm_targets_ex(param)

    cmp.remapAll([col.basis.radiance]*len(cmp.targets))
    
    print('-- end rendering --')
    return

def remap():
    print('\033[36mremapping\033[0m')

    cmp = composition.bi.Context()

    cmp.addImages(['nt', 'pt'])
    cmp.setTargets(targetMaterials)

    cmp.readFiles(param.nRay)

    cmp.remapAll(targetRemap)
    cmp.maskAll()

    print('-- end reamapping --')
    return

#render()
remap()