from . import composition as cmp

def hitsToImage(hits, renderpass, id, color):
    im = [0,0]*(3*renderpass.width*renderpass.height)
    for i in range(hits.size()):
        hit = hits.element(i)
        c = color(hit)
        im[3*hit.pixel  ] += hit.weight.x * c[0]
        im[3*hit.pixel+1] += hit.weight.y * c[1]
        im[3*hit.pixel+2] += hit.weight.z * c[2]
    for i in range(renderpass.width*renderpass.height):
        renderpass.set(id, i, im[3*i], im[3*i+1], im[3*i+2])