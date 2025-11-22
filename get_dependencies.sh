#!/usr/bin/env bash

conan install ./ \
	--output-folder=./ \
	--build=missing \
    --profile:host=default \
    --profile:build=default \
	--settings compiler.cppstd=17 \
    --generator CMakeDeps \
    --generator CMakeToolchain

# cmake -S ./$DEP_DIR/build/generators --preset=conan-default
# cmake --build ./$DEP_DIR/build --preset=conan-release


# we already have CMakePresets.json remove the conan generated one.
rm CMakeUserPresets.json

