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
cmp.scene.addMesh('left_proxi')
cmp.scene.addMesh('back_proxi')
#cmp.scene.addMesh('Suzanne')

cmp.scene.addTarget('target1')
cmp.scene.addTarget('target2')

cmp.scene.print()

time.sleep(0.2)
time0 = time.time()

cmp.pt_ref(rf, 500)

nRay = 16
hits0 = cmp.genHits_ex(0, nRay)
hits1 = cmp.genHits_ex(1, nRay)

cmp.hitsToImage(hits0, t0, col.basis.const(1, 1, 1))
cmp.hitsToImage(hits1, t1, col.basis.const(1, 1, 1))

#param = composition.core.PPMParam()
#cmp.ppm_ref(rf, param)

print(time.time() - time0)

terminate()