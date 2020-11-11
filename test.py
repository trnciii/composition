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


rf = passes.addLayer()
cmp.renderReference(passes, rf ,spp,scene)

nt = passes.addLayer()
cmp.renderNonTarget(passes, nt, spp, scene)

hits = cmp.collectHitpoints(w, h, nRay, 1, scene, target)
print("size ", hits.size())

# render weight as image
ds = passes.addLayer()
im_ds = [0.0]*(3*w*h)

for i in range(hits.size()):
	hit = hits.element(i)
	im_ds[3*hit.pixel  ] += hit.weight.x
	im_ds[3*hit.pixel+1] += hit.weight.y
	im_ds[3*hit.pixel+2] += hit.weight.z

for i in range(w*h):
	passes.set(ds, i, im_ds[3*i], im_ds[3*i+1], im_ds[3*i+2])
# end weight rendering


print("saved layers")
print( bin(cmp.writeAllPasses(passes, dir)) )