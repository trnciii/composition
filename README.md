# composition

A composite framework to stylize or edit material keeping global illumination effect.
It decomposes an image into effects of each target surface using photon mapping on the surface.

## process

It first traces suffix sub-path from eye to the target surface and store the hitpoint information.
Hitpoint information contains the throughput along each path as well as other necessary information for photon mapping.
The throughput represents the propagation of the radiance on the hitpoint to surrounding surface.

Then, the radiance on the collected hitpoints are estimated with photon mapping.
Now that we have both surface radiance and its bleeding into other surface.
We can edit the material with any info on the surface including its radiance, and can deliver it along its suffix path.

## dependencies

* [bledner](https://www.blender.org/) (bpy)
* [Boost.Python](https://www.boost.org/doc/libs/1_75_0/libs/python/doc/html/index.html)
* [glm](https://github.com/g-truc/glm)
* [stb](https://github.com/nothings/stb) (stb_image_write.h)

## todo

### soon
* embed necessary info in hitpoints
* manage rng seed

### future
* represent suffix path distribution with gaussian parameters or something

### later
* use matrix for spaces
* switch to range based raycasting

### pending
* parallelize hit to radiance conversion