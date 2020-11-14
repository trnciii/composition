import composition as cmp

dir = "result"

w = 512
h = 512
spp = 100
nRay = 16;
target = 5;

passes = cmp.RenderPasses(w,h)

scene = cmp.Scene()
if not cmp.createScene(scene):
	print("scene created")
else:
	print("failed to create a scene")


# id_rf = passes.addLayer()
# cmp.loadLayer(passes, id_rf, dir+"/reference")
# cmp.renderReference(passes, id_rf ,spp,scene)

id_nt = passes.addLayer()
cmp.loadLayer(passes, id_nt, dir + "/nontarget")
# cmp.renderNonTarget(passes, id_nt, spp, scene)


# hitpoints
# hits = cmp.collectHitpoints(1, w, h, nRay, 1, scene, target)
# hits.save(dir + "/hit")

hits = cmp.hitArray()
hits.load(dir + "/hit_1")


# render weights as an image
id_ds = passes.addLayer()
im_ds = [0.0]*(3*w*h)

for i in range(hits.size()):
	hit = hits.element(i)
	im_ds[3*hit.pixel  ] += hit.weight.x
	im_ds[3*hit.pixel+1] += hit.weight.y
	im_ds[3*hit.pixel+2] += hit.weight.z

for i in range(w*h):
	passes.set(id_ds, i, im_ds[3*i], im_ds[3*i+1], im_ds[3*i+2])
# end weight rendering


# # read / write test
# cmp.writeLayer(passes, id_ds, dir+"/distribution")

# load = passes.addLayer()
# cmp.loadLayer(passes, load, dir + "/distribution")


# ppm
R0 = 1.0
iteration = 100
nPhoton = 10000
alpha = 0.7

print("ppm with", iteration, "iterations")
cmp.progressivePhotonMapping(hits, R0, iteration, nPhoton, alpha, scene, target)


# image composition
im_target = [0.0]*(3*w*h)
for i in range(hits.size()):
	hit = hits.element(i)
	
	im_target[3*hit.pixel  ] += (hit.tau.x * hit.weight.x/iteration)
	im_target[3*hit.pixel+1] += (hit.tau.y * hit.weight.y/iteration)
	im_target[3*hit.pixel+2] += (hit.tau.z * hit.weight.z/iteration)

id_target = passes.addLayer()
for i in range(w*h):
	passes.set(id_target, i, im_target[3*i], im_target[3*i+1], im_target[3*i+2])

id_composed = passes.addLayer()
im_nt = cmp.getImage(passes, id_nt)
im_target = cmp.getImage(passes, id_target)
for i in range(w*h):
	x = im_target[4*i  ] + im_nt[4*i  ]
	y = im_target[4*i+1] + im_nt[4*i+1]
	z = im_target[4*i+2] + im_nt[4*i+2]
	passes.set(id_composed, i, x, y, z)

print("saved layers")
print( bin(cmp.writeAllPasses(passes, dir)) )