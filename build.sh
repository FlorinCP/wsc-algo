#!/bin/bash

# --- Configuration ---
CXX="g++-14" # Or your preferred modern C++ compiler
OUTPUT_NAME="sudoku_solver_pgo_lto"
CXX_STANDARD_FLAGS="-std=c++17"
# Add -flto to optimization flags
OPTIMIZATION_FLAGS="-O3 -march=native -flto"
LINKER_FLAGS="-lpthread"
PROFILE_DIR="."
# Add -Wall -Wextra for warnings during development
# WARNING_FLAGS="-Wall -Wextra" # Uncomment for development/debugging
WARNING_FLAGS=""

SOURCE_FILES=$(ls *.cpp 2> /dev/null) # Find all .cpp files

# Check if any .cpp files were found
if [ -z "$SOURCE_FILES" ]; then
  echo "Error: No .cpp files found in the current directory."
  exit 1
fi

echo "Found source files: $SOURCE_FILES"

# Clean previous profiling data (optional but recommended)
echo "Cleaning previous profile data..."
rm -f $PROFILE_DIR/*.gcda $PROFILE_DIR/*.gcno

echo "Step 1: Instrumented build (with LTO)..."
# Use OPTIMIZATION_FLAGS (includes -flto) for compile and link
$CXX $OPTIMIZATION_FLAGS -fprofile-generate=$PROFILE_DIR $CXX_STANDARD_FLAGS $WARNING_FLAGS \
    -o "$OUTPUT_NAME" \
    $SOURCE_FILES \
    $LINKER_FLAGS || { echo "Instrumented build failed."; exit 1; }

echo "Step 2: Running to generate profile data..."
echo "         (Make sure 'input.txt' is present and representative)"
# Run with a realistic workload
./"$OUTPUT_NAME" input.txt output_pgo_run.txt || { echo "Execution for profile data generation failed."; exit 1; }
# Clean up the potentially large output from the profile run
rm -f output_pgo_run.txt

echo "Step 3: Final optimized build using profile data (with LTO)..."
# Use OPTIMIZATION_FLAGS (includes -flto) for compile and link
$CXX $OPTIMIZATION_FLAGS -fprofile-use=$PROFILE_DIR -fprofile-correction $CXX_STANDARD_FLAGS $WARNING_FLAGS \
    -o "$OUTPUT_NAME" \
    $SOURCE_FILES \
    $LINKER_FLAGS || { echo "Final PGO+LTO build failed."; exit 1; }

# Clean up profiling data files
echo "Cleaning profile data files..."
rm -f $PROFILE_DIR/*.gcda $PROFILE_DIR/*.gcno

echo "Done! Final PGO+LTO binary: $OUTPUT_NAME"