#!/usr/bin/env bash
set -e

DEP_DIR="conan_dependencies"
mkdir -p "$DEP_DIR"

# Optional: local Conan cache
export CONAN_HOME="$(pwd)/.conan2"

echo "Using Conan cache at: $CONAN_HOME"
echo "Fetching dependencies into: $DEP_DIR"

# Ensure profile exists
if [[ ! -f "$CONAN_HOME/profiles/default" ]]; then
    echo "No Conan profile found. Creating one..."
    conan profile detect --force
fi

# Force C++17 settings in profile
#conan profile update settings.compiler.cppstd=17 default

# Detect conanfile
if [[ -f "conanfile.py" ]]; then
    CONANFILE="conanfile.py"
elif [[ -f "conanfile.txt" ]]; then
    CONANFILE="conanfile.txt"
else
    echo "ERROR: No conanfile.py or conanfile.txt found."
    exit 1
fi

conan install . \
    --output-folder="$DEP_DIR" \
    --build=missing \
    --profile:host=default \
    --profile:build=default

echo "Dependencies successfully installed in $DEP_DIR"
