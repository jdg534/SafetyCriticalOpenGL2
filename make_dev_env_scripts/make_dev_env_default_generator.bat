echo off
if exist ..\build rmdir -f ..\build

mkdir ..\build
cmake -s ../ -b ../build