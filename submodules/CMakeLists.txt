cmake_minimum_required(VERSION 3.30.0)

add_subdirectory(glm)
add_subdirectory(glfw)
add_subdirectory(glbinding)

# get the opengl SC 2.0 in: open_gl_registry/api/GLSC2
set(open_gl_sc_2_header_directory ${CMAKE_CURRENT_SOURCE_DIR}/open_gl_registry/api/GLSC2 CACHE INTERNAL "" FORCE)
