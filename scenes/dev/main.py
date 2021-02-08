import bpy
import time
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
param_preview.nRay = 64
param_preview.nPhoton = 100000
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
targetRemap = [target0(), target1(), rmFloor()]
#targetRemap[1] = col.basis.radiance

spheres = ['Sphere.001', 'Icosphere', 'Sphere']
meshes = ['Sphere.002', 'floor']

def render():
    cmp = composition.bi.Context()

    cmp.addImages(['nt', 'pt'])

    cmp.scene.create(spheres,meshes,targetMaterials)
    cmp.setTargets(targetMaterials)

    cmp.scene.print()
    time.sleep(0.1)

    cmp.pt_ref('pt', 100)
    cmp.save('pt', cmp.path+'im_pt')

    print('pt_nt');
    cmp.pt_nt('nt', 100)
    cmp.save('nt', cmp.path+"im_nontarget")
    print('')

#    cmp.ppm_targets(param)
    cmp.ppm_targets_ex(param)

    cmp.remapAll([col.basis.radiance]*len(cmp.targetNames))
    print('-- end renderingz --')

def remap():
    print('\033[36mremapping\033[0m')

    cmp = composition.bi.Context()

    cmp.addImages(['nt', 'pt'])
    cmp.setTargets(targetMaterials)

    cmp.loadFiles(param.nRay)

    cmp.remapAll(targetRemap)
#    cmp.maskAll()

    print('-- end reamapping --')
    return

render()
remap()