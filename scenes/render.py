import bpy
import time
import numpy as np
import composition
col = composition.color

def ls(a):
    for i in a:
        print(i)
    print()

targetMaterials = ['target1', 'target2']
nt = 'nt'

ramp_green = [
    (0, [0.03, 0.1, 0.03]),
    (0.07, [0.1, 0.5, 0.1 ]),
    (0.3 , [0.6, 0.8, 0.2]),
    (0.8 , [0.9, 1, 0.9]),
]

ramp_red = [
    (0, [0.1, 0.02, 0.02]),
    (0.3, [0.5, 0.1, 0.1]),
    (0.65, [0.8, 0.7, 0.2]),
    (1.5, [1, 1, 0.95])
]


def context():
    cmp = composition.bi.Context()

    cmp.bindImage(nt)
    for t in targetMaterials:
        if t not in bpy.data.images.keys():
            bpy.data.images.new(t, cmp.w, cmp.h)
        cmp.bindImage(t)

    cmp.scene.setCamera('Camera')

    env = composition.core.Material()
    env.type = composition.core.MtlType.emit
    env.color = composition.core.vec3(0, 0, 0)
    cmp.scene.setEnvironment(env)


    # add objects
    # light
    cmp.scene.addSphere('Sphere.001')

    # targets
    cmp.scene.addMesh('Sphere.002')
    cmp.scene.addSphere('Sphere')

    cmp.scene.addMesh('floor')
    cmp.scene.addMesh('left')
    cmp.scene.addMesh('back')
    cmp.scene.addMesh('right')

    # assign materials
    for t in targetMaterials:
        cmp.scene.addTarget(t)

    cmp.scene.print()
    return cmp

def background(cmp):
    #print('pt'); #cmp.pt_ref(rf, 500)
    #print('ppm ref'); cmp.ppm_ref(rf, param)

    print('pt_nt');
    cmp.pt_nt(nt, 200)
    cmp.save(nt, cmp.path+"/im_nontarget")
    print('')

def ppm_targets_ex(cmp, param):
    res = []
    for i in range(len(cmp.scene.data.targets)):
        print('collecting\033[32m exclusive\033[0m hitpoints on\033[33m', targetMaterials[i], '\033[0m')
        hits = cmp.genHits_ex(i, param.nRay)

        print('estimating radiance')
        cmp.ppm_radiance(hits, i, param)
        hits.save(cmp.path + "/hit_" + targetMaterials[i] + "_" + str(param.nRay) + "_ex")

        res.append(hits)
        print()

    return res

def ppm_targets(cmp, param):
    res = []
    for i in range(len(cmp.scene.data.targets)):
        print('collecting\033[32m all\033[0m hitpoints on\033[33m', targetMaterials[i], '\033[0m')
        hits = cmp.genHits(i, param.nRay)

        print('estimating radiance')
        cmp.ppm_radiance(hits, i, param)
        hits.save(cmp.path + "/hit_" + targetMaterials[i] +"_" + str(param.nRay) + "_all")

        res.append(hits)
        print()

    return res


def main():
    cmp = context()

    param_preview = composition.core.PPMParam()
    param_preview.nRay = 16
    param_preview.nPhoton = 1000
    param_preview.itr = 100

    param_final = composition.core.PPMParam()
    param_final.nRay = 256
    param_final.nPhoton = 100000
    param_final.itr = 10000

    param = param_preview


    time.sleep(0.2)

    
    background(cmp)


    hits_all = ppm_targets(cmp, param)
    hits_ex = ppm_targets_ex(cmp, param)

    for i in range(len(hits_ex)):
        print('converting\033[33m', targetMaterials[i], '\033[0m', end='')
        cmp.hitsToImage(hits_ex[i], targetMaterials[i], col.basis.radiance)
        print()

    return


print('\033[36mrender.py\033[0m')
main()

# delete variables
for name in dir():
    if not name.startswith('_'):
        # print(name)
        del globals()[name]
