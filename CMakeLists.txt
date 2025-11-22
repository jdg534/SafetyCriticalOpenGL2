cmake_minimum_required(VERSION 3.30.0)

project(OpenGl_SC_Playground)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# find package commands, all packages can be found in the conanfile.txt
# correct commands for the given library can be found on the conan center package for each library.
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glbinding REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(opengl_system REQUIRED)


add_subdirectory(third_party_code)
add_subdirectory(submodules)
add_subdirectory(source)

# Set artifact_executable as the VS start up project.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT artifact_executable)

