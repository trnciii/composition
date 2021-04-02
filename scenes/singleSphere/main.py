import bpy
import time
import composition
col = composition.color

# define ramps

param_preview = composition.core.PPMParam()
param_preview.nRay = 8
param_preview.nPhoton = 50000
param_preview.itr = 20

param = param_preview

def target0():
    ramp = 'ColorRamp.001'
    composition.bi.rampToImage(targetMaterials[0]+'_texture', ramp, 256, 16)
    remap = composition.bi.ramp(col.basis.sumRadianceRGB, ramp)
    return remap

targetMaterials = ['sphere']
targetRemap = {'sphere' : target0()}

spheres = ['Sphere.001', 'Sphere']
meshes = ['floor']

def render():
    cmp = composition.bi.Context()
    cmp.scene.create(spheres, meshes, targetMaterials)
    cmp.setTargets(targetMaterials)
    print(cmp.scene.data)
    time.sleep(0.1)

    cmp.pt_ref('pt', 1000)
    cmp.saveImage('pt')
    
    cmp.pt_nt('nt', 1000)
    cmp.saveImage('nt')
    print()

    for n in cmp.targetNames:
        cmp.genHits_ex(n, param.nRay, param.R0)
        cmp.genHits_all(n, param.nRay, param.R0)

    for k, h in cmp.hits.items():
        print(k)

        if k.type is composition.HitsType.EX:
            cmp.radiance_pt(h, 100000)
            # cmp.radiance_ppm(h, param)
        
        cmp.saveHits(k, param.nRay)

        print()

    cmp.remapAll({n : col.basis.radiance for n in cmp.targetNames})

    print('-- end rendering --')

def remap():
    print('\033[36mremapping\033[0m')

    cmp = composition.bi.Context()
    cmp.setTargets(targetMaterials)
    cmp.loadAll(param.nRay)

    cmp.remapAll(targetRemap)
    cmp.maskAll()

    print('-- end reamapping --')
    return

render()
remap()