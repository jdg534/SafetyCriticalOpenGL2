#!/usr/bin/env bash

conan install ./ \
	--output-folder=./ \
	--build=missing \
    --profile:host=default \
    --profile:build=default \
	--settings compiler.cppstd=17 \
    --generator CMakeDeps \
    --generator CMakeToolchain


