#!/bin/bash

cmake -S ./ --preset=windows-conan
cmake --build ./build
