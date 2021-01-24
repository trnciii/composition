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

def scene(scene):
    scene.setCamera('Camera')

    env = composition.core.Material()
    env.type = composition.core.MtlType.emit
    env.color = composition.core.vec3(0, 0, 0) # for photon mapping this has to be zero
    scene.setEnvironment(env)

    scene.addSphere('Sphere.001') # light
    scene.addMesh('Plane')
    scene.addMesh('Plane.001')
    scene.addMesh('Sphere.002')

def target0():
    ramp = 'ColorRamp'
    composition.bi.rampToImage(targetMaterials[0]+'_texture', ramp, 256, 16)
    
    return composition.bi.ramp(col.basis.sumRadianceRGB, ramp)

def target1():
    ramp = 'ColorRamp.001'
    composition.bi.rampToImage(targetMaterials[1]+'_texture', ramp, 256, 16)

    return composition.bi.ramp(col.basis.sumRadianceRGB, ramp)

targetMaterials = ['Material.002', 'Material.003']
nt = 'nt'

targetRemap = [target0, target1]

path = bpy.path.abspath('//../')

exec(open(path+'render.py').read())
exec(open(path+'composite.py').read())

# delete variables
for name in dir():
    if not name.startswith('_'):
        del globals()[name]