#!/bin/bash

rm -rf build

cmake -S ./ --preset=windows-conan
cmake --build ./build/
