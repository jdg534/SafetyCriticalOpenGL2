#!/bin/bash

cmake -S ./ --preset=conan-default
cmake --build ./build
