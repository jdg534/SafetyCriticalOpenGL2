#!/usr/bin/env bash

DEP_DIR="conan_dependencies"
rm -rf $DEP_DIR
mkdir -p $DEP_DIR
rm -rf .conan2/

# Optional: local Conan cache
export CONAN_HOME="$(pwd)/.conan2"

echo "Using Conan cache at: $CONAN_HOME"
echo "Fetching dependencies into: $DEP_DIR"

# Ensure profile exists
if [[ ! -f "$CONAN_HOME/profiles/default" ]]; then
    echo "No Conan profile found. Creating one..."
    conan profile detect --force
fi

conan install ./ \
    --output-folder="$DEP_DIR" \
    --build=missing \
    --profile:host=default \
    --profile:build=default \
	--settings compiler.cppstd=17 \
    --generator CMakeDeps \
    --generator CMakeToolchain

echo "Dependencies successfully installed in $DEP_DIR"

# we already have CMakePresets.json remove the conan generated one.
rm CMakeUserPresets.json

