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
# cmp.renderReference(passes, id_rf ,spp,scene)

# id_nt = passes.addLayer()
# cmp.renderNonTarget(passes, id_nt, spp, scene)

# hits = cmp.collectHitpoints(w, h, nRay, 1, scene, target)
# print("size ", hits.size())
# hits.save(dir + "/hit")

# render weight as image
hits_load = cmp.hitArray()
hits_load.load(dir + "/hit")
print("size (loaded)", hits_load.size())

id_ds = passes.addLayer()
im_ds = [0.0]*(3*w*h)

for i in range(hits_load.size()):
	hit = hits_load.element(i)
	im_ds[3*hit.pixel  ] += hit.weight.x
	im_ds[3*hit.pixel+1] += hit.weight.y
	im_ds[3*hit.pixel+2] += hit.weight.z

for i in range(w*h):
	passes.set(id_ds, i, im_ds[3*i], im_ds[3*i+1], im_ds[3*i+2])
# end weight rendering


# read / write test
cmp.writeLayer(passes, id_ds, dir+"/distribution")

load = passes.addLayer()
cmp.loadLayer(passes, load, dir + "/distribution")


print("saved layers")
print( bin(cmp.writeAllPasses(passes, dir)) )