import bpy
import time
import numpy as np
import composition
col = composition.color

def ls(a):
    for i in a:
        print(i)
    print()

def linfo(l):
    print(l.from_node)
    print(l.to_node)
    
def terminate():
    bpy.data.scenes["Scene"].node_tree.nodes["Alpha Over"].inputs[0].default_value = 0
    bpy.data.scenes["Scene"].node_tree.nodes["Alpha Over"].inputs[0].default_value = 1

    for name in dir():
        if not name.startswith('_'):
            print(name)
            del globals()[name]
    
    print("---- end ----")
    
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


path = bpy.path.abspath('//result') + '/'
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

# light
cmp.scene.addSphere('Sphere.001')
cmp.scene.addSphere('Sphere.002')
cmp.scene.addSphere('Sphere')

cmp.scene.addMesh('floor')
#cmp.scene.addMesh('left_proxi')
#cmp.scene.addMesh('back_proxi')
cmp.scene.addMesh('left')
cmp.scene.addMesh('back')
cmp.scene.addMesh('right')
#cmp.scene.addMesh('Suzanne')

cmp.scene.addTarget('target1')
cmp.scene.addTarget('target2')

cmp.scene.print()

time.sleep(0.2)
time0 = time.time()

#print('pt')
#cmp.pt_ref(rf, 500)

print('pt_nt')
cmp.pt_nt(nt, 50)

print(time0 - time.time())
terminate()


param = composition.core.PPMParam()
param.nRay = 64
param.nPhoton = 10000
param.itr = 1000

print('genHits')
hits0 = cmp.genHits_ex(0, param.nRay)
hits1 = cmp.genHits_ex(1, param.nRay)

print('radiance estimate')
cmp.ppm_radiance(hits0, 0, param)
cmp.ppm_radiance(hits1, 1, param)

hits0.save(path + "hits0_new_64")
hits1.save(path + "hits1_new_64")

ramp0 = col.Ramp(ramp_red, 'const')
ramp1 = col.Ramp(ramp_green, 'linear')

remap0 = col.basis.ramp(col.basis.sumRadianceRGB, ramp0.eval)
remap1 = col.basis.ramp(col.basis.sumRadianceRGB, ramp1.eval)

print('conversion')
cmp.hitsToImage(hits0, t0, remap0)
cmp.hitsToImage(hits1, t1, remap1)

print(time.time() - time0)

terminate()