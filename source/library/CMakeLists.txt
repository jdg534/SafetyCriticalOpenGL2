cmake_minimum_required(VERSION 3.30.0)

file(GLOB_RECURSE artifact_library_headers *.h)
file(GLOB_RECURSE artifact_library_source *.cpp)

add_library(artifact_library ${artifact_library_headers} ${artifact_library_source})

set(artifact_library_header_directory ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)
