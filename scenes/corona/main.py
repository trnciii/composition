import bpy
import composition
col = composition.color

param_preview = composition.core.PPMParam()
param_preview.nRay = 16
param_preview.nPhoton = 1000
param_preview.itr = 200

param_final = composition.core.PPMParam()
param_final.nRay = 256
param_final.nPhoton = 100000
param_final.itr = 10000

param = param_preview


def target0():
    ramp = 'ColorRamp'
    composition.bi.rampToImage(targetMaterials[0]+'_texture', ramp, 256, 16)
    
    return composition.bi.ramp(col.basis.sumRadianceRGB, ramp)

def target1():
    ramp = 'ColorRamp.001'
    composition.bi.rampToImage(targetMaterials[1]+'_texture', ramp, 256, 16)

    return composition.bi.ramp(col.basis.sumRadianceRGB, ramp)

targetMaterials = ['Material.002', 'Material.003']
spheres = ['Sphere.001']
meshes = ['Plane', 'Plane.001', 'Sphere.002']

targetRemap = [target0(), target1()]

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

    cmp.ppm_targets(param)
    cmp.ppm_targets_ex(param)

    cmp.remapAll([col.basis.radiance]*len(cmp.targets))
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