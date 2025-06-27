#!/bin/bash

# Configuration
SOURCE_DIR="source"
BUILD_DIR="build_for_clang_tidy_analysis"

# Ensure compile_commands.json exists
if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "Generating compile_commands.json with CMake..."
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B "$BUILD_DIR"
fi

# Find and analyze only files in source/
find "$SOURCE_DIR" \( -name "*.cpp" -o -name "*.cc" -o -name "*.cxx" -o -name "*.c" -o -name "*.h" -o -name "*.hpp" \) | while read -r file; do
    echo "Running clang-tidy on $file"
    clang-tidy "$file" -p "$BUILD_DIR"
done
