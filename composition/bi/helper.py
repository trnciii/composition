import bpy
import math
from ..core import composition

def rampToImage(key, ramp):
    img = bpy.data.images[key]
    w, h = img.size
    print("ramp image size", w, h)
    px = [0.0]*4*w*h
    d = 2**(math.ceil(math.log(ramp.data[-1][0], 2)))

    for i in range(w):
        c = ramp.eval(d*i/w)
        
        for j in range(h):
            idx = j*w + i
            px[4*idx  ] = c.x
            px[4*idx+1] = c.y
            px[4*idx+2] = c.z
            px[4*idx+3] = 1
    
    img.pixels = px

def sliceImage(key, y):
    im = bpy.data.images[key]
    w, h = im.size
    
    p = [[0.]*3 for i in range(w)]
    y = max(0, min(h-1, int(y*h)))
    for x in range(w):
        i = y*w+x
        p[x][0] = im.pixels[4*i  ]
        p[x][1] = im.pixels[4*i+1]
        p[x][2] = im.pixels[4*i+2]
    return p

def findShader(key):
    tree = bpy.data.materials[key].node_tree
    links = tree.links
    nodes = tree.nodes
    names = [n.name for n in nodes]
    
    shaders = [] 
    for l in links:
        t = l.to_node
        f = l.from_node
        if type(t) is bpy.types.ShaderNodeOutputMaterial:
            if t.is_active_output and l.to_socket.name == 'Surface':
                shaders.append(f)
    
    if len(shaders)>0:
        return shaders[0]

def createMaterial(key):
    shader = findShader(key)
    
    if(shader.type == 'EMISSION'):
        c = shader.inputs[0].default_value
        s = shader.inputs[1].default_value
        
        m = composition.Material()
        m.type = composition.MtlType.emit
        m.color = composition.vec3(c[0]*s, c[1]*s, c[2]*s)
        return m
        
    if(shader.type == 'BSDF_DIFFUSE'):
        c = shader.inputs['Color'].default_value
        
        m = composition.Material()
        m.type = composition.MtlType.lambert
        m.color = composition.vec3(c[0], c[1], c[2])
        return m
    
    if(shader.type == 'BSDF_GLOSSY'):
        c = shader.inputs['Color'].default_value
        a = shader.inputs['Roughness'].default_value
        
        m = composition.Material()
        m.type = composition.MtlType.glossy
        m.color = composition.vec3(c[0], c[1], c[2])
        m.alpha = a
        return m
    
    print('could not convert material')