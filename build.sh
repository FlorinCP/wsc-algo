#!/bin/bash

# --- Configuration ---
# Compiler to use
CXX="g++-14"
# Output executable name
OUTPUT_NAME="sudoku_solver_pgo"
# Standard C++ flags
CXX_STANDARD_FLAGS="-std=c++17"
# Optimization flags (used in both steps)
OPTIMIZATION_FLAGS="-O3 -march=native"
# Linker flags
LINKER_FLAGS="-lpthread"
# Profile data directory/prefix ('.' means current directory)
PROFILE_DIR="."

SOURCE_FILES="main.cpp sudoku_solver.cpp worker.cpp"

# Check if any .cpp files were found
if [ -z "$SOURCE_FILES" ]; then
  echo "Error: No .cpp files found in the current directory."
  exit 1
fi

echo "Found source files: $SOURCE_FILES"

echo "Step 1: Instrumented build..."
$CXX $OPTIMIZATION_FLAGS -fprofile-generate=$PROFILE_DIR $CXX_STANDARD_FLAGS \
    -o "$OUTPUT_NAME" \
    $SOURCE_FILES \
    $LINKER_FLAGS || { echo "Instrumented build failed."; exit 1; }

echo "Step 2: Running to generate profile data..."
echo "         (Make sure 'input.txt' is present in this directory)"
./"$OUTPUT_NAME" || { echo "Execution for profile data generation failed."; exit 1; }

echo "Step 3: Final optimized build using profile data..."
$CXX $OPTIMIZATION_FLAGS -fprofile-use=$PROFILE_DIR -fprofile-correction $CXX_STANDARD_FLAGS \
    -o "$OUTPUT_NAME" \
    $SOURCE_FILES \
    $LINKER_FLAGS || { echo "Final PGO build failed."; exit 1; }

echo "Done! Final PGO binary: $OUTPUT_NAME"

rm -f $PROFILE_DIR/*.gcda $PROFILE_DIR/*.gcno