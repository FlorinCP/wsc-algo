@echo off
set CXX=cl.exe
set OUTPUT_NAME=sudoku_solver.exe
set CXX_STANDARD_FLAGS=/std:c++17 /EHsc
set OPTIMIZATION_FLAGS=/Ox /GL /arch:AVX2 /fp:fast /MD
set LINKER_FLAGS_INSTRUMENT=/GENPROFILE /DEBUG /INCREMENTAL:NO
set LINKER_FLAGS_FINAL=/LTCG:PGO /INCREMENTAL:NO

set SOURCE_FILES=src/main.cpp src/sudoku_solver.cpp src/worker.cpp

set HEADER_FILES_DIR=headers

if "%SOURCE_FILES%"=="" (
  echo Error: SOURCE_FILES variable is not set in the script.
  exit /b 1
)

echo Found source files: %SOURCE_FILES%
echo Using Compiler: %CXX%
echo Optimization Flags: %OPTIMIZATION_FLAGS%
echo WARNING: Using /arch:AVX2 - compiled executable requires a CPU supporting AVX2 instructions!
echo ---

set PGO_DATABASE=%OUTPUT_NAME:.exe=.pgd%

set PDB_DATABASE=%OUTPUT_NAME:.exe=.pdb%

echo Step 1: Instrumented build for profiling...
%CXX% %CXX_STANDARD_FLAGS% %OPTIMIZATION_FLAGS% /Fe:"%OUTPUT_NAME%" %SOURCE_FILES% /I %HEADER_FILES_DIR% /link %LINKER_FLAGS_INSTRUMENT% /PDB:"%PDB_DATABASE%"
if errorlevel 1 (
    echo Instrumented build failed.
    exit /b 1
)
echo ---

echo Step 2: Running to generate profile data (%PGO_DATABASE%)...
echo          (Make sure 'input.txt' contains representative workloads!)
echo          (The quality of optimization depends heavily on this step.)
.\%OUTPUT_NAME%
if errorlevel 1 (
    echo Execution for profile data generation failed.
    echo Note: The PGO database [%PGO_DATABASE%] might still have been created/updated partially.
) else (
    if not exist "%PGO_DATABASE%" (
      echo WARNING: Profile database file [%PGO_DATABASE%] not found after execution.
      echo          The program might have exited too quickly or crashed before writing data.
      echo          PGO optimization in the next step might be ineffective.
    )
)
echo ---

echo Step 3: Final optimized build using profile data [%PGO_DATABASE%]...
%CXX% %CXX_STANDARD_FLAGS% %OPTIMIZATION_FLAGS% /Fe:"%OUTPUT_NAME%" %SOURCE_FILES% /I %HEADER_FILES_DIR% /link %LINKER_FLAGS_FINAL% /PDB:"%PDB_DATABASE%"
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

echo Cleanup complete.

exit /b 0