# SafetyCriticalOpenGL2
This is for Practicing OpenGL SC 2.0 code.

Note that rather than focusing on MISRA C++ 2023, ontop of the C++17 standard.
HIC++ is used, this is due to it being supported by clang-tidy without having to deal with licences and subscriptions.

HIC++ validation is done via a CI script, if the script passes then the relevent code can be considered as HIC++ complient.

## Areas to address

3D rendering.
	Optionally with lighting.
	Optionally with texture mapping.
2D text rendering with bitmap fonts (using shaders and a font atlas).

### Should Address areas

Skeletal animation of a skinned mesh, pre computed forward kinematic animations only. (IF this can be done while being HIC++ complient)
Text rendering using SDF / MSDF font atlases. (IF this can be done while being HIC++ complient)


# How to build

The artifact is run via the artifact_executable.

To build this first checkout the submodules with:

git submodules update --init

To create the build directory run:

cmake -S ./ -B build

Assuming Visual Studio, is your default cmake generator simply run the start up project.

