cmake_minimum_required(VERSION 3.30.0)

project(OpenGl_SC_Playground)

find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

add_subdirectory(third_party_code)
add_subdirectory(submodules)
add_subdirectory(source)

# Set artifact_executable as the VS start up project.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT artifact_executable)

