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

param_preview = composition.core.PPMParam()
param_preview.nRay = 16
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

    return composition.bi.ramp(u, ramp)

    # ramp_bin = col.RampData([(0, [0., 0., 1.]), (0.5, [1., 0., 0.])], 'const')
    # l =  bpy.data.objects['Sphere.001'].location
    # return col.basis.cel_diffuse(ramp_bin, list(l))


targetMaterials = ['target1']
targetRemap = {'target1' : target0()}

#targetRemap = [col.basis.const(0.8, 0.7, 0.2)]
#targetRemap = [col.basis.radiance]

spheres = ['Sphere', 'Sphere.001']
meshes = ['Sphere.002','right.001']

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
            # cmp.radiance_pt(h, 1000)
            cmp.radiance_ppm(h, param)
        
        cmp.saveHits(k, param.nRay)

        print()


    cmp.remapAll({n : col.basis.radiance for n in cmp.targetNames})
    
    return


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

#saveImages(bpy.path.abspath('//intermidiate/'), targetMaterials)