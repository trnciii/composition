import bpy
import time
import composition
col = composition.color


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
    ramp = 'ColorRamp'

    composition.bi.rampToImage(targetMaterials[0]+'_texture', ramp, 256, 16)
    remap = composition.bi.ramp(col.basis.sumRadianceRGB, ramp)

    return remap

def target1():
    ramp = 'ColorRamp.001'
    
    def u(hit):
        return col.basis.sumRadianceRGB(hit)*250
    
    composition.bi.rampToImage(targetMaterials[1]+'_texture', ramp, 256, 16)
    remap = composition.bi.ramp(u, ramp)
    return remap

def target2():
    ramp = 'ColorRamp.002'
    
    def u(hit):
        return col.basis.sumRadianceRGB(hit)*4
    
    composition.bi.rampToImage(targetMaterials[1]+'_texture', ramp, 256, 16)
    remap = composition.bi.ramp(u, ramp)
    return remap


targetMaterials = ['diffuse', 'water', 'earth']
targetRemap = [target0(), target1(), target2()]

spheres = [
    'sky',
    'source',
]
meshes = [
    'Suzanne',
    'earth',
    'water',
    'Plane',
    'Plane.002',
]

def render():
    print('\033[36mrendering\033[0m')
    
    cmp = composition.bi.Context()

    cmp.addImages(['nt', 'pt'])
    cmp.scene.create(spheres, meshes, targetMaterials)
    cmp.setTargets(targetMaterials)

    cmp.scene.print()
    time.sleep(0.1)

    cmp.pt_ref('pt', 2000)
    cmp.save('pt', cmp.path+'im_pt')

    print('pt_nt');
    cmp.pt_nt('nt', 2000)
    cmp.save('nt', cmp.path+"im_nontarget")
    print()    

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

    cmp.loadFiles(param.nRay)

    cmp.remapAll(targetRemap)
    cmp.maskAll()

    print('-- end reamapping --')
    return

render()
remap()