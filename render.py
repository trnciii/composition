import bpy
import time
import numpy as np
import composition
col = composition.color

def ls(a):
    for i in a:
        print(i)
    print()
    
def terminate():
    bpy.data.scenes["Scene"].node_tree.nodes["Alpha Over"].inputs[0].default_value = 0
    bpy.data.scenes["Scene"].node_tree.nodes["Alpha Over"].inputs[0].default_value = 1

    for name in dir():
        if not name.startswith('_'):
            print(name)
            del globals()[name]
    

t0 = 'target1'
t1 = 'target2'
t0s = 'target1s'
t1s = 'target2s'
m0 = 'mask1'
m1 = 'mask2'
d0 = 'depth1'
d1 = 'depth2'
nt = 'nt'
rf = 'reference'
tx0 = 'texture1'
tx1 = 'texture2'
    
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

    cmp.bindImage(rf)
    cmp.bindImage(nt)
    cmp.bindImage(t0)
    cmp.bindImage(t1)

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
    #cmp.scene.addMesh('left_proxi')
    #cmp.scene.addMesh('back_proxi')
    cmp.scene.addMesh('left')
    cmp.scene.addMesh('back')
    cmp.scene.addMesh('right')
    #cmp.scene.addMesh('Suzanne')


    # assign materials
    cmp.scene.addTarget('target1')
    cmp.scene.addTarget('target2')

    cmp.scene.print()
    return cmp

def background(cmp):
    #print('pt'); #cmp.pt_ref(rf, 500)
    #print('ppm ref'); cmp.ppm_ref(rf, param)

    print('pt_nt');
    cmp.pt_nt(nt, 200)
    cmp.save(nt, cmp.path+"/im_nontarget")

def ppm_ex(cmp, param):
    print('collect target hitpoints')
    hits0 = cmp.genHits_ex(0, param.nRay)
    hits1 = cmp.genHits_ex(1, param.nRay)

    print('radiance estimate')
    cmp.ppm_radiance(hits0, 0, param)
    hits0.save(cmp.path + "/hit_1_" + str(param.nRay) + "_ex")

    cmp.ppm_radiance(hits1, 1, param)
    hits1.save(cmp.path + "/hit_2_" + str(param.nRay) + "_ex")

    return hits0, hits1

def ppm(cmp, param):
    print('collect target hitpoints')
    hits0 = cmp.genHits(0, param.nRay)
    hits1 = cmp.genHits(1, param.nRay)

    print('radiance estimate')
    cmp.ppm_radiance(hits0, 0, param)
    hits0.save(cmp.path + "/hit_1_" + str(param.nRay) + "_all")

    cmp.ppm_radiance(hits1, 1, param)
    hits1.save(cmp.path + "/hit_2_" + str(param.nRay) + "_all")

    return hits0, hits1

def main():

    cmp = context()

    param_preview = composition.core.PPMParam()
    param_preview.nRay = 16
    param_preview.nPhoton = 10000
    param_preview.itr = 100

    param_final = composition.core.PPMParam()
    param_final.nRay = 256
    param_final.nPhoton = 100000
    param_final.itr = 10000

    param = param_preview


    time.sleep(0.2)
    time0 = time.time()

    
    background(cmp)

    hits0 = composition.core.Hits()
    hits1 = composition.core.Hits()
    
    hits0, hits1 = ppm(cmp, param)
    hits0, hits1 = ppm_ex(cmp, param)


    ramp0 = col.RampData(ramp_red, 'const')
    ramp1 = col.RampData(ramp_green, 'linear')

    remap0 = col.basis.ramp(col.basis.sumRadianceRGB, ramp0.eval)
    remap1 = col.basis.ramp(col.basis.sumRadianceRGB, ramp1.eval)

    print('conversion')
    cmp.hitsToImage(hits0, t0, col.basis.radiance)
    cmp.hitsToImage(hits1, t1, col.basis.radiance)

    print(time.time() - time0)

    terminate()
    return

main()