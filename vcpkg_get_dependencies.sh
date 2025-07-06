#!/bin/bash

package_type_to_get=x64-windows-static

./submodules/vcpkg/vcpkg install zlib:$package_type_to_get
./submodules/vcpkg/vcpkg install libpng:$package_type_to_get
./submodules/vcpkg/vcpkg install rapidjson:$package_type_to_get

