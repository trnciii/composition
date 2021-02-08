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
targetRemap = [target0()]

spheres = ['Sphere.001', 'Sphere']
meshes = ['floor']

def render():
    cmp = composition.bi.Context()

    cmp.addImages(['nt', 'pt'])
    cmp.scene.create(spheres, meshes, targetMaterials)
    cmp.setTargets(targetMaterials)

    cmp.scene.print()
    time.sleep(0.1)

    cmp.pt_ref('pt', 1000)
    cmp.save('pt', cmp.path+'im_pt')

    print('pt_nt');
    cmp.pt_nt('nt', 1000)
    cmp.save('nt', cmp.path+"im_nontarget")
    print('')

    cmp.ppm_targets(param)
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
    cmp.maskAll()

    print('-- end reamapping --')
    return

render()
remap()