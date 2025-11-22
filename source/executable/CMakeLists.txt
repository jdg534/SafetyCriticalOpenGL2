cmake_minimum_required(VERSION 3.30.0)

file(GLOB_RECURSE artifact_executable_headers_and_source *.h *.cpp)
add_executable(artifact_executable ${artifact_executable_headers_and_source})

target_link_libraries(artifact_executable
    PRIVATE
        artifact_library
        ${glfw3_LIBRARIES}
		${assimp_LIBRARIES}
        ${glbinding_LIBRARIES}
		${opengl_system_LIBRARIES}
)

target_include_directories(artifact_executable
    PUBLIC
        ${artifact_library_header_directory}
)

add_custom_command(TARGET artifact_executable POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:artifact_executable>/../assets"
)

get_target_property(loc_gl glbinding::glbinding LOCATION)
get_target_property(loc_khr glbinding::KHRplatform LOCATION)
message(STATUS "glbinding: ${loc_gl}")
message(STATUS "KHRplatform: ${loc_khr}")

