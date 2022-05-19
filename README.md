In this repository, We provide the implementations of 2 methods in the article [Global Illumination-Aware Stylised Shading](https://onlinelibrary.wiley.com/doi/10.1111/cgf.14397) and [Non-photorealistic Radiance Remapping](https://dl.acm.org/doi/10.1145/3388770.3407395).
They both are stylized rendering algorithm that keeps global illumination effects: The stylized object can be seen in reflections, refraction, or color bleeding, as well as the texture coordinate considers shadow or caustics on the target.
One method is based on the path tracing, and the other is on the [Progressive Photon Mapping](https://dl.acm.org/doi/10.1145/1409060.1409083)(PPM).
Please access to the papers for more information about the features of each method.



## Components

|file or directory | |
|:-|:-|
| `/src/kernel.cpp`         | All important functions for our methods. Descriptions will be added as comments. |
| `/src/main/ppm_based.cpp` | stylization based on PPM written as the second method in [Global Illumination-Aware Stylised Shading](https://onlinelibrary.wiley.com/doi/10.1111/cgf.14397). |
| `/src/main/pt_based.cpp`  | stylization based on path tracing. written in [Non-photorealistic Radiance Remapping](https://dl.acm.org/doi/10.1145/3388770.3407395) and the first method of [Global Illumination-Aware Stylised Shading](https://onlinelibrary.wiley.com/doi/10.1111/cgf.14397). |
| `/src/main/ref_ppm.cpp`   | PPM implementation with some modifications. We don't distinguish caustic surface and diffuse surface. |
| `/src/main/ref_pt.cpp`    | Path tracing implementation. |
| `/src/interface`          | Python interface using [Pybind11](https://github.com/pybind/pybind11)|
| `/composition/`           | Python package mainly to work with Blender. Put this under Blender's addon directory. |
| `/scenes/`                | sample Blender scenes and scripts. |




## Requirement

All programs require:

* C++ 17 supported compiler
* OpenMP (Tested with gcc's. You can disable it from CMake configuration.)
* CMake

Additionally, the Python interface requires:

* Python 3.9 (You need the same version as Blender's)



## Build

First, initiate submodules (glm and pybind11).

```shell
$ git submodule update --init
```

Then, cmake and build the programs.
We assume building under `/build` directory.

```shell
$ mkdir build
$ cd build
$ cmake ..
$ make
```

The built executables could be found in `build/bin`, and python core module is put in `/composition/core/` directory.
For example, `$ bin/ppm_based` to run the PPM based stylization.
The programs in `build/bin` save the resulting images in `./result`.


## Contribute

Questions, discussions, requests are welcome.
Contact me via any github features or doi.kohei.682@s.kyushu-u.ac.jp
