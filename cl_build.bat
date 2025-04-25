@echo off
REM --- Configuration ---
REM Compiler (automatically found in Developer Command Prompt)
set CXX=cl.exe
REM Output executable name
set OUTPUT_NAME=sudoku_solver.exe
REM Standard C++ flags (Visual Studio specific)
set CXX_STANDARD_FLAGS=/std:c++17 /EHsc
REM Optimization flags (Visual Studio specific)
REM /Ox  - Aggressive speed optimizations (includes /O2, /Ob2, /Ot, etc.)
REM /GL  - Whole Program Optimization (Required for PGO/LTCG)
REM /arch:AVX2 - Enable AVX2 instructions. WARNING: Reduces compatibility with older CPUs!
REM              Remove or change to /arch:AVX or /arch:SSE2 for broader compatibility.
REM /fp:fast   - Faster floating-point model (less strict).
REM /MD  - Link with dynamic multi-threaded runtime library.
set OPTIMIZATION_FLAGS=/Ox /GL /arch:AVX2 /fp:fast /MD
REM Linker flags for Step 1 (Instrumentation)
set LINKER_FLAGS_INSTRUMENT=/PROFILE
REM Linker flags for Step 3 (Final Optimized Build)
REM /LTCG:PGO - Use Profile Guided Optimization data.
REM /INCREMENTAL:NO - Disable incremental linking for potentially better optimization.
set LINKER_FLAGS_FINAL=/LTCG:PGO /INCREMENTAL:NO

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
echo Optimization Flags: %OPTIMIZATION_FLAGS%
echo WARNING: Using /arch:AVX2 - compiled executable requires a CPU supporting AVX2 instructions!
echo ---

REM PGO generates profile data files (.pgc, .pgd)
set PGO_DATABASE=%OUTPUT_NAME:.exe=.pgd%

REM --- Step 1: Instrumented build ---
echo Step 1: Instrumented build for profiling...
REM Compile with optimizations and /GL, link with /PROFILE
%CXX% %CXX_STANDARD_FLAGS% %OPTIMIZATION_FLAGS% /Fe:"%OUTPUT_NAME%" %SOURCE_FILES% /link %LINKER_FLAGS_INSTRUMENT% /Fd:"%OUTPUT_NAME%.pdb"
if errorlevel 1 (
    echo Instrumented build failed.
    exit /b 1
)
echo ---

REM --- Step 2: Running to generate profile data ---
echo Step 2: Running to generate profile data (%PGO_DATABASE%)...
echo          (Make sure 'input.txt' contains representative workloads!)
echo          (The quality of optimization depends heavily on this step.)
REM Run the instrumented executable. It creates .pgc files merged into .pgd on exit.
.\%OUTPUT_NAME%
if errorlevel 1 (
    echo Execution for profile data generation failed.
    echo Note: The PGO database (%PGO_DATABASE%) might still have been created/updated partially.
) else (
    REM Check if the PGD file was actually created (program might have exited instantly)
    if not exist "%PGO_DATABASE%" (
      echo WARNING: Profile database file (%PGO_DATABASE%) not found after execution.
      echo          The program might have exited too quickly or crashed before writing data.
      echo          PGO optimization in the next step might be ineffective.
    )
)
echo ---

REM --- Step 3: Final optimized build using profile data ---
echo Step 3: Final optimized build using profile data (%PGO_DATABASE%)...
REM Re-compile with same optimization flags (/Ox /GL /arch:AVX2), link with /LTCG:PGO
%CXX% %CXX_STANDARD_FLAGS% %OPTIMIZATION_FLAGS% /Fe:"%OUTPUT_NAME%" %SOURCE_FILES% /link %LINKER_FLAGS_FINAL% /Fd:"%OUTPUT_NAME%.pdb"
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