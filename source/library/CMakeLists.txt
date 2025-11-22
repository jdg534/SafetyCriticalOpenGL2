cmake_minimum_required(VERSION 3.30.0)

file(GLOB_RECURSE artifact_library_headers *.h)
file(GLOB_RECURSE artifact_library_source *.cpp)

add_library(artifact_library ${artifact_library_headers} ${artifact_library_source})

set(artifact_library_header_directory ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)

# egl and sc includes.
target_include_directories(artifact_library PRIVATE ${egl_header_directory})
target_include_directories(artifact_library PRIVATE ${open_gl_sc_2_header_directory})

# conan dependencies
target_link_libraries(artifact_library PUBLIC PNG::PNG)
target_link_libraries(artifact_library PUBLIC glm::glm)
target_link_libraries(artifact_library PUBLIC glfw)
target_link_libraries(artifact_library PUBLIC glbinding::glbinding-aux)
target_link_libraries(artifact_library PUBLIC assimp::assimp)
target_link_libraries(artifact_library PUBLIC ZLIB::ZLIB)
target_link_libraries(artifact_library PUBLIC rapidjson)
