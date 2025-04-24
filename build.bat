@echo off
REM Ensures commands themselves are not printed to the console.

REM --- Configuration ---
REM Compiler to use (ensure g++.exe or equivalent is in your PATH)
set CXX=g++.exe
REM Consider changing this if you installed MinGW/GCC elsewhere or use a specific version like x86_64-w64-mingw32-g++.exe
REM set CXX=g++-14.exe  REM <-- Or specifically this if you have it named like that

REM Output executable name (Windows standard is .exe)
set OUTPUT_NAME=sudoku_solver_pgo.exe
REM Standard C++ flags
set CXX_STANDARD_FLAGS=-std=c++17
REM Optimization flags (used in both steps)
set OPTIMIZATION_FLAGS=-O3 -march=native
REM Linker flags (pthread library is usually needed for std::thread with MinGW)
set LINKER_FLAGS=-lpthread
REM Profile data directory/prefix ('.' means current directory)
set PROFILE_DIR=.

REM Source files (space-separated)
set SOURCE_FILES=main.cpp sudoku_solver.cpp worker.cpp

REM --- Sanity Check ---
REM Basic check if the SOURCE_FILES variable is set. Does NOT check if files actually exist.
if "%SOURCE_FILES%"=="" (
  echo Error: SOURCE_FILES variable is empty. Please define source files.
  exit /b 1
)

echo Found source files: %SOURCE_FILES%
echo ---

REM --- Build Steps ---

echo Step 1: Instrumented build...
%CXX% %OPTIMIZATION_FLAGS% -fprofile-generate=%PROFILE_DIR% %CXX_STANDARD_FLAGS% -o "%OUTPUT_NAME%" %SOURCE_FILES% %LINKER_FLAGS%

REM Check if the compilation succeeded. %ERRORLEVEL% is non-zero on failure.
if %ERRORLEVEL% neq 0 (
    echo Instrumented build failed. Exit code: %ERRORLEVEL%
    exit /b 1
)
echo ---

echo Step 2: Running to generate profile data...
echo          (Make sure 'input.txt' is present in this directory)

REM Execute the instrumented program. Use .\ to ensure it runs from the current directory.
.\%OUTPUT_NAME%

REM Check if the program ran successfully.
if %ERRORLEVEL% neq 0 (
    echo Execution for profile data generation failed. Exit code: %ERRORLEVEL%
    exit /b 1
)
echo ---

echo Step 3: Final optimized build using profile data...
%CXX% %OPTIMIZATION_FLAGS% -fprofile-use=%PROFILE_DIR% -fprofile-correction %CXX_STANDARD_FLAGS% -o "%OUTPUT_NAME%" %SOURCE_FILES% %LINKER_FLAGS%

REM Check if the final compilation succeeded.
if %ERRORLEVEL% neq 0 (
    echo Final PGO build failed. Exit code: %ERRORLEVEL%
    exit /b 1
)
echo ---

echo Cleaning up profile data files (*.gcda, *.gcno)...
REM Delete the generated profile data files.
REM /Q enables quiet mode (no confirmation prompt).
REM > nul 2>&1 suppresses "File Not Found" errors if the files don't exist.
del /Q "%PROFILE_DIR%\*.gcda" > nul 2>&1
del /Q "%PROFILE_DIR%\*.gcno" > nul 2>&1
echo ---

echo Done! Final PGO binary: %OUTPUT_NAME%

REM Exit with success code 0
exit /b 0