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