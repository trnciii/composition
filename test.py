import bpy
import importlib
import numpy as np
import composition
importlib.reload(composition)
col = composition.color

path = "C:\\Users\\Rinne\\Drive\\workshop\\composition\\result\\"
t1 = 'target1'
t2 = 'target2'
t1l = 'target1_linear'
t2l = 'target2_linear'
m1 = 'mask1'
m2 = 'mask2'
d1 = 'depth1'
d2 = 'depth2'
nt = 'nt'
rf = 'referemce'

def hitsToDepth(context, hits, key):
    depth = [0]*(context.renderpass.width*context.renderpass.height)
    count = [0]*(context.renderpass.width*context.renderpass.height)
    for i in range(hits.size()):
        hit = hits.element(i)
        depth[hit.pixel] += hit.depth
        count[hit.pixel] += 1
    for i in range(context.renderpass.width*context.renderpass.height):
#        d = depth[i]/count[i] if count[i] else 0
        d = depth[i]/128
        context.renderpass.set(context.bind[key], i, d, d, d)

def purity(rad, ch):
    a = (rad[0] + rad[1] + rad[2])/3
    p = rad[ch]-a
    if 0<p:
        return p*10
    else:
        return 0

def cel(hit):
    l = [-hit.p.x, -hit.p.y, 6-hit.p.z]
    dot = l[0]*hit.n.x + l[1]*hit.n.y + l[2]*hit.n.z
    dot = 0.5*dot + 0.5
    rad = col.radiance(hit)
    m = min(rad)
    M = max(rad)
    g = purity(rad, 1)      
#    return [g, g, g]
    c = [0.3, 0.1, 0.03] if dot<0 else [0.8, 0.6, 0.1]
    return [c[0], (1-g)*c[1]+g, c[2]]

def mask(cmp, hits, key, nRay):
    length = cmp.renderpass.width*cmp.renderpass.height
    count = [0]*3*length

    for i in range(hits.size()):
        hit = hits.element(i)
        count[hit.pixel] += 1

    for i in range(length):
        cmp.renderpass.set(cmp.bind[key], i, count[i]/nRay, 0, 0)
    
    cmp.copyImage(key)
    
def setAlpha(cmp, key_color, key_alpha, key_out):
    ps = cmp.renderpass
    im = composition.core.getImage(ps, cmp.bind[key_color])
    a = composition.core.getImage(ps, cmp.bind[key_alpha])
    for i in range(ps.width*ps.height):
        im[4*i+3] = a[4*i]
#    bpy.data.images[key_out].pixels = im
    bpy.data.images[key_out].pixels = im
    
def depth(cmp, hits, key, nRay):
    length = cmp.renderpass.width*cmp.renderpass.height
    d = [0]*3*length
    count = [0]*3*length
    max = 0
    for i in range(hits.size()):
        hit = hits.element(i)
        d[hit.pixel] += hit.depth
        count[hit.pixel] += 1
        max = hit.depth if hit.depth > max else max
    for i in range(length):
        if count[i] > 0:
            cmp.renderpass.set(cmp.bind[key], i, d[i]/nRay, 0, 0)
    cmp.copyImage(key)
    return max

print("---- start ----")

hits1_ex = composition.core.Hits()
hits2_ex = composition.core.Hits()
hits1_total = composition.core.Hits()
hits2_total = composition.core.Hits()


#hits1_ex.load(path + "hits1")
#hits2_ex.load(path + "hits2")
hits1_total.load(path + "hits1_all")
hits2_total.load(path + "hits2_all")

cmp = composition.Context()
cmp.bindImage(t1)
cmp.bindImage(t2)
cmp.bindImage(m1)
cmp.bindImage(m2)

#mask(cmp, hits1_ex, m1, 512)
mask(cmp, hits1_total, m1, 64)
#cmp.mask(hits1_total, m1, 64)
#cmp.mask(hits2_total, m2, 64)

#mask(cmp, hits2_ex, m2, 128)
mask(cmp, hits2_total, m2, 64)

cmp.hitsToImage(hits1_total, t1, col.normal)
cmp.hitsToImage(hits2_total, t2, col.normal)
#cmp.hitsToImage(hits2, t2, col.radiance)

setAlpha(cmp, t1, m1, t1)
setAlpha(cmp, t2, m2, t2)

del cmp
del hits1_ex
del hits2_ex
del hits1_total, hits2_total

print("---- end ----")