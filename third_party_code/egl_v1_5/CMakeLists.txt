cmake_minimum_required(VERSION 3.30.0)

file(GLOB_RECURSE egl_library_headers *.h)

set(egl_header_directory ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)

