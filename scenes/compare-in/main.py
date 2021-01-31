import bpy
import time
import composition
col = composition.color

def saveImages(path, layers):
    names = ['nt', 'pt']
    names = names+[layers]
    names = names+[l+'_mask' for l in layers]
    names = names+[l+'_depth' for l in layers]
    names = names+[l+'_texture' for l in layers]
    
    for n in names:
        if n in bpy.data.images.keys():
            file = path+n+'.png'
            print('save', file)
            bpy.data.images[n].save_render(file)

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

param_preview = composition.core.PPMParam()
param_preview.nRay = 4
param_preview.nPhoton = 20000
param_preview.itr = 100

param_final = composition.core.PPMParam()
param_final.nRay = 32
param_final.nPhoton = 200000
param_final.itr = 10000

param = param_preview

def target0():
    ramp = 'ColorRamp'

    def u(hit):
        r = col.basis.sumRadianceRGB(hit)
        r = r*0.3
        r = r**0.5
        return r

    composition.bi.rampToImage(targetMaterials[0]+'_texture', ramp, 256, 16)
    remap = composition.bi.ramp(u, ramp)
    return remap

targetMaterials = ['target1']
targetRemap = [target0()]
#targetRemap = [col.basis.const(0.8, 0.7, 0.2)]
#targetRemap = [col.basis.radiance]

spheres = ['Sphere', 'Sphere.001', 'Sphere.002']
meshes = ['Plane']

def render():
    print('\033[36mrendering\033[0m')
    cmp = composition.bi.Context()

    cmp.addImages(['nt', 'pt'])
    cmp.scene.create(spheres, meshes, targetMaterials)
    cmp.setTargets(targetMaterials)

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

    cmp.loadFiles(param.nRay)

    cmp.remapAll(targetRemap)
    cmp.maskAll()

    print('-- end reamapping --')
    return

render()
remap()

#saveImages(bpy.path.abspath('//result/intermidiate/'), targetMaterials)