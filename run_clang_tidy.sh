#!/usr/bin/env bash

cmake --preset=static-analysis

BUILD_DIR="./static_analysis"
COMPILE_DB="$BUILD_DIR/compile_commands.json"

while IFS= read -r file; do
	windows_style_path=$(cygpath -m "$file")
    clang-tidy "$windows_style_path" -p "$BUILD_DIR"
done < <(jq -r '.[].file | select(contains("submodules") | not)' "$COMPILE_DB")

