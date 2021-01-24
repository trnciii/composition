import bpy
import composition
col = composition.color

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
param_preview.nRay = 16
param_preview.nPhoton = 1000
param_preview.itr = 200

param_final = composition.core.PPMParam()
param_final.nRay = 256
param_final.nPhoton = 100000
param_final.itr = 10000

param = param_preview

def scene(scene):
    scene.setCamera('Camera')

    env = composition.core.Material()
    env.type = composition.core.MtlType.emit
    env.color = composition.core.vec3(0, 0, 0) # for photon mapping this has to be zero
    scene.setEnvironment(env)

    scene.addSphere('Sphere.001') # light
    scene.addMesh('Sphere.002')
    scene.addSphere('Sphere')
    scene.addMesh('floor')
    scene.addMesh('left')
    scene.addMesh('back')
    scene.addMesh('right')

def target0():
    ramp = col.RampData(ramp_green2, 'const')    
    # print(ramp)
    # remap = col.basis.ramp(col.basis.sumRadianceRGB, ramp)
    # ramp = 'ColorRamp'
    # l =  bpy.data.objects['Sphere.001'].location

    composition.bi.rampToImage(targetMaterials[0]+'_texture', ramp, 256, 16)

    remap = composition.bi.ramp(col.basis.sumRadianceRGB, ramp)
    # remap = col.basis.cel_diffuse(ramp, list(l))

    return remap

def target1():
    # ramp = col.RampData(ramp_red0, 'const')
    # print(ramp)
    # ramp = composition.bi.sliceImage('a.png', 0.5)
    ramp = 'ColorRamp.001'
    
    composition.bi.rampToImage(targetMaterials[1]+'_texture', ramp, 256, 16)

    remap = composition.bi.ramp(col.basis.sumRadianceRGB, ramp)
    # remap = col.mix(remap, col.basis.radiance, 0.25)
    # remap = col.mul(remap, col.basis.radiance)
    # remap = col.mul(remap, col.basis.const(6, 6, 6))
    
    return remap

targetMaterials = ['target1', 'target2']
nt = 'nt'

targetRemap = [target0(), target1()]

path = bpy.path.abspath('//../')

#exec(open(path+'render.py').read())
exec(open(path+'composite.py').read())

# delete variables
for name in dir():
    if not name.startswith('_'):
        del globals()[name]