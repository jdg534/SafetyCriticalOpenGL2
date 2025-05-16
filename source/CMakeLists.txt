cmake_minimum_required(VERSION 3.30.0)

file(GLOB_RECURSE initial_source_files *.h *.cpp)
add_executable(initial_executable ${initial_source_files})
