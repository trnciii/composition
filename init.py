import composition as cmp

print("functions")
print(dir(cmp))
print("")

print("Scene")
print(dir(cmp.Scene))
print("")

w = 512
h = 512
spp_pt = 100

print("     w = " + str(w))
print("     h = " + str(h))
print("spp_pt = " + str(spp_pt))

scene = cmp.Scene()
if not cmp.createScene(scene):
	print("scene created")
else:
	print("failed to create a scene")

cmp.renderReference(w,h,spp_pt,scene)