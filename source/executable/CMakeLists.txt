cmake_minimum_required(VERSION 3.30.0)

file(GLOB_RECURSE artifact_executable_headers_and_source *.h *.cpp)
add_executable(artifact_executable ${artifact_executable_headers_and_source})

target_link_libraries(artifact_executable PUBLIC artifact_library)

target_include_directories(artifact_executable PUBLIC ${artifact_library_header_directory})
