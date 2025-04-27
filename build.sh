#!/bin/bash

CXX="g++-14"
OUTPUT_NAME="sudoku_solver"
SOURCE_DIR="src"
HEADER_DIR="headers"
PROFILE_DIR="."

CXX_STANDARD_FLAGS="-std=c++17"
OPTIMIZATION_FLAGS="-O3 -march=native -flto"
LINKER_FLAGS="-lpthread"
WARNING_FLAGS="-Wall -Wextra"

SOURCE_FILES=$(ls "$SOURCE_DIR"/*.cpp 2> /dev/null)

if [ -z "$SOURCE_FILES" ]; then
  echo "Error: No .cpp files found in the '$SOURCE_DIR' directory."
  exit 1
fi

if [ ! -d "$HEADER_DIR" ]; then
    echo "Warning: Header directory '$HEADER_DIR' not found."
fi

echo "Found source files in '$SOURCE_DIR':"
for f in $SOURCE_FILES; do echo "  - $f"; done
echo "Using header directory: '$HEADER_DIR'"


echo "Cleaning previous build artifacts and profile data..."
rm -f "$OUTPUT_NAME" "$PROFILE_DIR"/*.gcda "$PROFILE_DIR"/*.gcno

echo "Step 1: Instrumented build (with LTO)..."
"$CXX" $OPTIMIZATION_FLAGS -fprofile-generate="$PROFILE_DIR" $CXX_STANDARD_FLAGS $WARNING_FLAGS \
    -o "$OUTPUT_NAME" \
    $SOURCE_FILES \
    $LINKER_FLAGS \
    -I"$HEADER_DIR" || { echo "Instrumented build failed."; exit 1; }

echo "Step 2: Running to generate profile data..."
echo "         (Make sure 'input.txt' is present and representative)"
./"$OUTPUT_NAME" input.txt output_pgo_run.txt || { echo "Execution for profile data generation failed."; exit 1; }
rm -f output_pgo_run.txt

echo "Step 3: Final optimized build using profile data (with LTO)..."
"$CXX" $OPTIMIZATION_FLAGS -fprofile-use="$PROFILE_DIR" -fprofile-correction $CXX_STANDARD_FLAGS $WARNING_FLAGS \
    -o "$OUTPUT_NAME" \
    $SOURCE_FILES \
    $LINKER_FLAGS \
    -I"$HEADER_DIR" || { echo "Final PGO+LTO build failed."; exit 1; }

echo "Cleaning profile data files..."
rm -f "$PROFILE_DIR"/*.gcda "$PROFILE_DIR"/*.gcno

echo "Done! Final PGO+LTO binary: $OUTPUT_NAME"