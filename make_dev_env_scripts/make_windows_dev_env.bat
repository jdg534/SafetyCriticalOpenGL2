echo off
if exist ..\build rmdir -f ..\build
cd ..
cmake --preset=windows-static
