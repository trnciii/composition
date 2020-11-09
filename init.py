import composition as cmp

dir = "result"

w = 512
h = 512
spp_pt = 100

passes = cmp.RenderPasses(w,h)

scene = cmp.Scene()
if not cmp.createScene(scene):
	print("scene created")
else:
	print("failed to create a scene")


reference = passes.addLayer()
image_reference = cmp.renderReference(passes, reference ,spp_pt,scene)

nt = passes.addLayer()
image_nt = cmp.renderNonTarget(passes, nt, spp_pt, scene)


print("saved layers")
print( bin(cmp.writePasses(passes, dir)) )