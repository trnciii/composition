import composition as cmp

def visualizeDistribution(hits, renderpass, id):
	im = [0,0]*(3*renderpass.width*renderpass.height)
	for i in range(hits.size()):
		hit = hits.element(i)
		im[3*hit.pixel  ] += hit.throuput.x
		im[3*hit.pixel+1] += hit.throuput.y
		im[3*hit.pixel+2] += hit.throuput.z
	for i in range(w*h):
		renderpass.set(id, i, im[3*i], im[3*i+1], im[3*i+2])

def color1(hit):
	u = (hit.tau.x + hit.tau.y + hit.tau.z)/hit.iteration
	return [0.9, 0.2, 0.2] if u>2 else [0.5, 0.1, 0.1]

def color2(hit):
	u = (hit.tau.x + hit.tau.y + hit.tau.z)/hit.iteration
	return [0.2, 0.9, 0.2] if u>2 else [0.1, 0.5, 0.1]

def hitsToColor(hits, renderpass, id, color):
	im = [0,0]*(3*renderpass.width*renderpass.height)
	for i in range(hits.size()):
		hit = hits.element(i)
		c = color(hit)
		im[3*hit.pixel  ] += hit.throuput.x * c[0]
		im[3*hit.pixel+1] += hit.throuput.y * c[1]
		im[3*hit.pixel+2] += hit.throuput.z * c[2]
	for i in range(w*h):
		renderpass.set(id, i, im[3*i], im[3*i+1], im[3*i+2])

def hitsToRaw(hits, renderpass, id):
    im = [0,0]*(3*renderpass.width*renderpass.height)
    for i in range(hits.size()):
        hit = hits.element(i)
        im[3*hit.pixel  ] += hit.throuput.x * hit.tau.x / hit.iteration
        im[3*hit.pixel+1] += hit.throuput.y * hit.tau.y / hit.iteration
        im[3*hit.pixel+2] += hit.throuput.z * hit.tau.z / hit.iteration
    for i in range(w*h):
        renderpass.set(id, i, im[3*i], im[3*i+1], im[3*i+2])


print("---- START ----")

dir = "C:\\Users\\Rinne\\Drive\\workshop\\composition\\result"

w = 512
h = 512
spp = 2000

target1 = 0
target2 = 1

renderpass = cmp.RenderPass(w,h)
scene = cmp.Scene()
cmp.createScene(scene)
	
###############################################

# reference 
# id_rf = renderpass.addLayer()
# cmp.renderReference(renderpass, id_rf ,spp,scene)
# print("reference image renderd")

# # non target
# id_nt = renderpass.addLayer()
# cmp.renderNonTarget(renderpass, id_nt ,spp, scene)
# print("non target image renderd")

# hitpoint
nRay = 16
hits1 = cmp.collectHitpoints(1, w, h, nRay, 1, scene, target1)
hits2 = cmp.collectHitpoints(1, w, h, nRay, 1, scene, target2)

id_ds1 = renderpass.addLayer()
id_ds2 = renderpass.addLayer()
visualizeDistribution(hits1, renderpass, id_ds1)
visualizeDistribution(hits2, renderpass, id_ds2)

# ppm to update hits
R0 = 0.5
alpha = 0.6
iteration = 10
nPhoton = 100000

print("ppm with", iteration, "iterations")
cmp.progressivePhotonMapping(hits1, R0, iteration, nPhoton, alpha, scene, target1)
cmp.progressivePhotonMapping(hits2, R0, iteration, nPhoton, alpha, scene, target2)

print("compose target contribution")
id_t1 = renderpass.addLayer()
id_t2 = renderpass.addLayer()
hitsToColor(hits1, renderpass, id_t1, color1)
hitsToColor(hits2, renderpass, id_t2, color2)

id_raw1 = renderpass.addLayer()
id_raw2 = renderpass.addLayer()
hitsToRaw(hits1, renderpass, id_raw1)
hitsToRaw(hits2, renderpass, id_raw2)

print(bin( cmp.writeAllPass(renderpass, dir) ))

print("---- END ----")