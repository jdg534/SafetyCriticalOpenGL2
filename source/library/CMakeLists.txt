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
target_link_libraries(artifact_library PUBLIC glm::glm) # need the include directories
target_link_libraries(artifact_library PUBLIC glfw) # need the include directories.
target_link_libraries(artifact_library PUBLIC glbinding::KHRplatform) # need the include directories
target_link_libraries(artifact_library PUBLIC glbinding::glbinding) # need the include directories
target_link_libraries(artifact_library PUBLIC assimp::assimp) # need the include directories
target_link_libraries(artifact_library PUBLIC ZLIB::ZLIB)
target_link_libraries(artifact_library PUBLIC rapidjson) # need the include directories
target_link_libraries(artifact_library PUBLIC OpenGL::GL)

# to get where the variables are defined look up the files generated in: SafetyCriticalOpenGL2/conan_dependencies/build/generators/<generated file name>
# the file name will be similar to the package name.


target_link_libraries(artifact_library PUBLIC ${assimp_LIBRARIES})

target_link_libraries(artifact_library PUBLIC ${glfw3_LIBRARIES})

target_include_directories(artifact_library
    PUBLIC
        ${glm_INCLUDE_DIRS} # from: SafetyCriticalOpenGL2/conan_dependencies/build/generators/glm-config.cmake
        ${glfw3_INCLUDE_DIRS} # from: SafetyCriticalOpenGL2/conan_dependencies/build/generators/glfw3-config.cmake
        ${assimp_INCLUDE_DIRS} # from: SafetyCriticalOpenGL2/conan_dependencies/build/generators/assimp-config.cmake
        ${glbinding_INCLUDE_DIRS} # from: SafetyCriticalOpenGL2/conan_dependencies/build/generators/glbinding-config.cmake
        ${RapidJSON_INCLUDE_DIRS} # from: SafetyCriticalOpenGL2/conan_dependencies/build/generators/RapidJSONConfig.cmake       
)

target_include_directories(artifact_library
    PUBLIC
		${egl_header_directory}
		${open_gl_sc_2_header_directory}
)

