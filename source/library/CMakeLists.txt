cmake_minimum_required(VERSION 3.30.0)

file(GLOB_RECURSE artifact_library_headers *.h)
file(GLOB_RECURSE artifact_library_source *.cpp)

set(artifact_library_header_directory ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)

add_library(artifact_library ${artifact_library_headers} ${artifact_library_source})

target_include_directories(artifact_library
    PUBLIC
        ${glm_INCLUDE_DIRS}
        ${glfw3_INCLUDE_DIRS}
        ${assimp_INCLUDE_DIRS}
        ${glbinding_INCLUDE_DIRS}
        ${RapidJSON_INCLUDE_DIRS}
        ${egl_header_directory}
        ${open_gl_sc_2_header_directory}
)

target_link_libraries(artifact_library
    PUBLIC
        PNG::PNG
        glm::glm
        glfw              # or glfw3::glfw if using CMake package
        glbinding::KHRplatform
        glbinding::glbinding
        assimp::assimp
        ZLIB::ZLIB
        rapidjson
        OpenGL::GL
)


