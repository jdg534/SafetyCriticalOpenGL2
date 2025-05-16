echo off
if exist ..\build rmdir -f ..\build

mkdir ..\build
cmake -S ../ -B ../build