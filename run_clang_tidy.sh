#!/usr/bin/env bash
set -euo pipefail

cmake --preset=static-analysis

BUILD_DIR="./static_analysis"
COMPILE_DB="$BUILD_DIR/compile_commands.json"

FAIL=0

while IFS= read -r file; do
    clang-tidy "$file" -p "$BUILD_DIR" --warnings-as-errors='*' || FAIL=1
done < <(jq -r '.[].file | select(contains("submodules") | not)' "$COMPILE_DB")

exit $FAIL