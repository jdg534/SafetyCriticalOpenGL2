echo off
if exist ..\build rmdir -f ..\build

mkdir ..\build
cmake -S ../ -B ../build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
