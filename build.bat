@echo off
REM --- Configuration ---
REM Compiler to use (make sure it's in your PATH)
set CXX=g++
REM Output executable name (Windows uses .exe)
set OUTPUT_NAME=sudoku_solver_pgo.exe
REM Standard C++ flags
set CXX_STANDARD_FLAGS=-std=c++17
REM Optimization flags (used in both steps)
set OPTIMIZATION_FLAGS=-O3 -march=native
REM Linker flags (assuming MinGW g++ which understands -lpthread)
set LINKER_FLAGS=-lpthread
REM Profile data directory/prefix ('.' means current directory)
set PROFILE_DIR=.

REM --- Source Files ---
REM Explicitly list your source files here
set SOURCE_FILES=main.cpp sudoku_solver.cpp worker.cpp

REM Basic check if source files are set (not checking existence here)
if "%SOURCE_FILES%"=="" (
  echo Error: SOURCE_FILES variable is not set in the script.
  exit /b 1
)

echo Found source files: %SOURCE_FILES%
echo ---

echo Step 1: Instrumented build...
%CXX% %OPTIMIZATION_FLAGS% -fprofile-generate=%PROFILE_DIR% %CXX_STANDARD_FLAGS% -o "%OUTPUT_NAME%" %SOURCE_FILES% %LINKER_FLAGS%
if errorlevel 1 (
    echo Instrumented build failed.
    exit /b 1
)
echo ---

echo Step 2: Running to generate profile data...
echo          (Make sure 'input.txt' is present in this directory)
REM Execute the instrumented program
.\%OUTPUT_NAME%
if errorlevel 1 (
    echo Execution for profile data generation failed.
    exit /b 1
)
echo ---

echo Step 3: Final optimized build using profile data...
%CXX% %OPTIMIZATION_FLAGS% -fprofile-use=%PROFILE_DIR% -fprofile-correction %CXX_STANDARD_FLAGS% -o "%OUTPUT_NAME%" %SOURCE_FILES% %LINKER_FLAGS%
if errorlevel 1 (
    echo Final PGO build failed.
    exit /b 1
)
echo ---

echo Done! Final PGO binary: %OUTPUT_NAME%
echo ---

echo Cleaning up profile data files...
del /f /q "%PROFILE_DIR%\*.gcda" > nul 2>&1
del /f /q "%PROFILE_DIR%\*.gcno" > nul 2>&1

echo Cleanup complete.

exit /b 0