cmake_minimum_required(VERSION 3.30.0)

file(GLOB_RECURSE artifact_executable_headers_and_source *.h *.cpp)
add_executable(artifact_executable ${artifact_executable_headers_and_source})

target_link_libraries(artifact_executable PUBLIC artifact_library)

target_include_directories(artifact_executable PUBLIC ${artifact_library_header_directory})

# copy assets from the asserts folder to the binary ouput dir as a post build step.
add_custom_command(TARGET artifact_executable POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:artifact_executable>/../assets"
)

# When getting to packaging the executable, look into conditional logic if need differing behaviour.
