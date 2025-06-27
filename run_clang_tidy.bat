@echo off
setlocal enabledelayedexpansion

REM Config
set SOURCE_DIR=source
set BUILD_DIR=build_for_clang_tidy_analysis

REM Generate compile_commands.json if it doesn't exist
if not exist %BUILD_DIR%\compile_commands.json (
    echo Generating compile_commands.json with CMake...
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B %BUILD_DIR%
)

REM Analyze files in source/
for %%F in (cpp cc cxx c h hpp) do (
    for /R "%SOURCE_DIR%" %%f in (*.^%%F) do (
        echo Running clang-tidy on %%f
        clang-tidy "%%f" -p "%BUILD_DIR%"
    )
)

endlocal
