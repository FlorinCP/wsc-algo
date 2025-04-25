@echo off
REM --- Configuration ---
REM Compiler (automatically found in Developer Command Prompt)
set CXX=cl.exe
REM Output executable name
set OUTPUT_NAME=sudoku_solver_pgo.exe
REM Standard C++ flags (Visual Studio specific)
set CXX_STANDARD_FLAGS=/std:c++17 /EHsc
REM Optimization flags (Visual Studio specific)
REM /O2  - Maximize speed (equivalent to GCC's -O2/O3)
REM /GL  - Whole Program Optimization (Required for PGO instrumentation/linking)
REM /MD  - Link with dynamic multi-threaded runtime library (common choice)
set OPTIMIZATION_FLAGS=/O2 /GL /MD
REM Linker flags (Generally fewer explicit flags needed than GCC)
REM Additional libs could be added here if needed, e.g., MyLib.lib
set LINKER_FLAGS=

REM --- Source Files ---
REM Explicitly list your source files here
set SOURCE_FILES=main.cpp sudoku_solver.cpp worker.cpp

REM Basic check if source files are set (not checking existence here)
if "%SOURCE_FILES%"=="" (
  echo Error: SOURCE_FILES variable is not set in the script.
  exit /b 1
)

echo Found source files: %SOURCE_FILES%
echo Using Compiler: %CXX%
echo ---

REM PGO generates profile data files (.pgc, .pgd)
set PGO_DATABASE=%OUTPUT_NAME:.exe=.pgd%

REM --- Step 1: Instrumented build ---
echo Step 1: Instrumented build for profiling...
REM /link /PROFILE tells the linker to prepare for PGO training.
REM /Fd: adds program database file name - helps avoid conflicts if used later
%CXX% %CXX_STANDARD_FLAGS% %OPTIMIZATION_FLAGS% /Fe:"%OUTPUT_NAME%" %SOURCE_FILES% /link /PROFILE /Fd:"%OUTPUT_NAME%.pdb" %LINKER_FLAGS%
if errorlevel 1 (
    echo Instrumented build failed.
    exit /b 1
)
echo ---

REM --- Step 2: Running to generate profile data ---
echo Step 2: Running to generate profile data (%PGO_DATABASE%)...
echo          (Make sure 'input.txt' is present in this directory)
REM Run the instrumented executable. It will create .pgc files which are merged into the .pgd file on exit.
.\%OUTPUT_NAME%
if errorlevel 1 (
    echo Execution for profile data generation failed.
    echo Note: The PGO database (%PGO_DATABASE%) might still have been created/updated partially.
    REM Decide if you want to stop here or try the final build anyway
    REM exit /b 1
)
echo ---

REM --- Step 3: Final optimized build using profile data ---
echo Step 3: Final optimized build using profile data (%PGO_DATABASE%)...
REM /link /LTCG:PGO tells the linker to use the generated PGO data.
REM /USEPROFILE is often implicitly handled by LTCG:PGO but can be added explicitly to cl.exe compile line if needed.
%CXX% %CXX_STANDARD_FLAGS% %OPTIMIZATION_FLAGS% /Fe:"%OUTPUT_NAME%" %SOURCE_FILES% /link /LTCG:PGO /Fd:"%OUTPUT_NAME%.pdb" %LINKER_FLAGS%
if errorlevel 1 (
    echo Final PGO build failed.
    exit /b 1
)
echo ---

echo Done! Final PGO binary: %OUTPUT_NAME%
echo ---

echo Cleaning up intermediate PGO and build files...
del /f /q *.pgc > nul 2>&1
del /f /q *.pgd > nul 2>&1
del /f /q *.ilk > nul 2>&1
REM Optionally delete PDB files too if not needed for debugging
REM del /f /q *.pdb > nul 2>&1

echo Cleanup complete.

exit /b 0