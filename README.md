Within this repository, you will find implementations of two stylized rendering algorithms that preserve global illumination effects as detailed in two articles: [Global Illumination-Aware Stylised Shading](https://onlinelibrary.wiley.com/doi/10.1111/cgf.14397) and [Non-photorealistic Radiance Remapping](https://dl.acm.org/doi/10.1145/3388770.3407395). These methods allow for the stylized object to be observed in reflections, refractions, color bleeding, as well as texture coordinates that take into account shadows or caustics on the target. One of the techniques is based on path tracing, while the other is an extension of [the Progressive Photon Mapping](https://dl.acm.org/doi/10.1145/1409060.1409083) (PPM) algorithm. For further information about the features of each method, please refer to the respective academic papers.


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

First, initialize submodules (glm and pybind11).

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
Contact me via any github features.
