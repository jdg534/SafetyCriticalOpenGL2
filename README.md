# SafetyCriticalOpenGL2

This is for Practicing OpenGL SC 2.0 code / the closest equivalent without running in certified hardware (OpenGL ES 2.0).

Note that rather than focusing on MISRA C++ 2023, ontop of the C++17 standard. [HI-C++](https://en.wikipedia.org/wiki/High_Integrity_C%2B%2B) is used, this is due to it being supported by clang-tidy without having to deal with licences and subscriptions.

HI-C++ validation is done via a CI script, if the script passes then the relevent code can be considered as HI-C++ complient.

C++17 is used as the langage standard.

## Areas addressed

- Rendering of static meshes.
- Rendering of terrain.
- Rendering of textured quads.
- Rendering of bitmap fonts.
- OpenGL ES 2.0 is used to ensure similarity to OpenGL SC 2.0 covering:
	- OpenGL ES 2.0 drawing context used at initilaisation time.
	- All glsl shaders use `#version 100` which is the only accepted shader version supporting under OpenGL SC 2.0.

## TODOs

Some remaining improvement exist.

### Must Address areas

- Update the clang-tidy useage to use a [compilation database](https://cmake.org/cmake/help/latest/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html) and jq to iterate over the files being compiled via a .sh script.
- Bugfix: T-Junction detection and elimination. In the vertex & index buffer population code for the terrain.
- Custom memory allocators.
    - Initialisation phase, no limit, but won't allow allocations once initialisation finishes.
	- Running phase, fixed size, can allow allocations after Initialisation.
- (Once all of the above are addressed) Update the code to be HI C++ complient.

### Should Address areas (on revisiting the project)

- Rendering of rigged geometry, forward kinematics only.
- Text rendering using SDF / MSDF font atlases. (IF this can be done while being HI-C++ complient)
- Improvement: Vertex buffer optimisation, reusing vertices.
- Removal of git submodule dependencies in favour of dependencies with vcpkg packages or conan package (not a mix of both).

### Could address areas

- Conan dependencies instead of vcpkg dependencies, with removal of using git submodules.

# How to build

Assuming building for windows simply run:
```bash
git submodules update --init
cmake --preset=windows-static
```
In the generated `build/` folder you will find `OpenGl_SC_Playground.sln`, open it and attempt to run `artifact_executable`.

## Packaging

Packaging of this library to be consumable as a binary dependency will be considered if requested.
