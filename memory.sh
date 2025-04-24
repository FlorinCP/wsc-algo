#!/bin/bash

# --- Configuration ---
SAMPLE_INTERVAL=0.2 # Seconds between memory checks
# --- End Configuration ---

# Check for the command to run
if [ $# -eq 0 ]; then
  echo "Usage: $0 <command_to_run> [args...]"
  echo "Example: $0 ./sudoku_solver_pgo input.txt output.txt 6"
  exit 1
fi

# The command and its arguments are passed directly
COMMAND_TO_RUN=("$@")

# --- Timing Setup ---
# Check for high-resolution date command (GNU date for %N)
# On macOS, this often requires `brew install coreutils` and using `gdate`
if command -v gdate > /dev/null; then
    DATE_CMD="gdate"
    NS_SUPPORT=1
    # echo "[Monitor] Using gdate for high-resolution timing."
elif date --version > /dev/null 2>&1 && date --version | grep -q GNU; then
    DATE_CMD="date"
    NS_SUPPORT=1
    # echo "[Monitor] Using GNU date for high-resolution timing."
else
    DATE_CMD="date"
    NS_SUPPORT=0
    echo "[Monitor] Warning: High-resolution timing (%N) not detected. Using seconds only."
    echo "[Monitor]          (On macOS, install coreutils and use 'gdate' for ms precision)."
fi
# --- End Timing Setup ---


echo "[Monitor] Starting command: ${COMMAND_TO_RUN[@]}"

# --- Start Timer ---
if [ "$NS_SUPPORT" -eq 1 ]; then
    start_ns=$($DATE_CMD +%s%N)
else
    start_s=$($DATE_CMD +%s)
fi
# --- End Start Timer ---

# Execute the command in the background
"${COMMAND_TO_RUN[@]}" &
# Get the Process ID (PID) of the background command
TARGET_PID=$!

if [ -z "$TARGET_PID" ]; then
    echo "[Monitor] Error: Failed to start the command in the background."
    exit 1
fi
echo "[Monitor] Target PID: $TARGET_PID"


peak_rss_kb=0
is_running=1
measurement_count=0

echo "[Monitor] Monitoring RSS (Peak KB)..."

# Loop while the process *might* be running
while [ $is_running -eq 1 ]; do
    # Check if the process still exists using kill -0 (doesn't actually kill)
    if ! kill -0 "$TARGET_PID" > /dev/null 2>&1; then
        is_running=0
        # echo "[Monitor] Process $TARGET_PID finished." # Optional debug
        break # Exit the loop immediately
    fi

    # Get Resident Set Size (RSS) in Kilobytes using ps
    current_rss_kb=$(ps -o rss= -p "$TARGET_PID" 2>/dev/null | awk 'NR>0{print $1; exit}')

    # Check if we got a valid number
    if [[ "$current_rss_kb" =~ ^[0-9]+$ ]]; then
         measurement_count=$((measurement_count + 1))
        # Update peak value if current is greater
        if [ "$current_rss_kb" -gt "$peak_rss_kb" ]; then
            peak_rss_kb=$current_rss_kb
            # Optional: print updates
            # printf "[Monitor] Peak RSS: %'d KB\r" "$peak_rss_kb"
        fi
    else
        : # Ignore temporary ps errors, process existence checked above
    fi

    # Wait before next sample
    sleep "$SAMPLE_INTERVAL"
done

# echo # Newline after potential \r prints
echo "[Monitor] Monitoring stopped."

# Wait for the target process to *fully* terminate and get its exit code
wait "$TARGET_PID"
target_exit_code=$?

# --- Stop Timer ---
if [ "$NS_SUPPORT" -eq 1 ]; then
    end_ns=$($DATE_CMD +%s%N)
else
    end_s=$($DATE_CMD +%s)
fi
# --- End Stop Timer ---

# --- Calculate Duration ---
duration_ms=0 # Initialize duration in milliseconds
if [ "$NS_SUPPORT" -eq 1 ]; then
    # Check if timestamps are valid numbers before calculating
    if [[ "$start_ns" =~ ^[0-9]+$ && "$end_ns" =~ ^[0-9]+$ ]]; then
        duration_ns=$((end_ns - start_ns))
        # Calculate milliseconds using integer division
        duration_ms=$((duration_ns / 1000000))
        duration_string="${duration_ms} ms"
    else
        duration_string="Error calculating high-res duration"
    fi
else
    # Check if timestamps are valid numbers
    if [[ "$start_s" =~ ^[0-9]+$ && "$end_s" =~ ^[0-9]+$ ]]; then
        duration_s=$((end_s - start_s))
        # Convert seconds to milliseconds
        duration_ms=$((duration_s * 1000))
        duration_string="${duration_ms} ms (from seconds)" # Indicate lower precision
    else
        duration_string="Error calculating duration"
    fi
fi
# --- End Calculate Duration ---


echo "----------------------------------------"
echo "Command Execution Finished"
echo "  Exit Code: $target_exit_code"
echo "  Execution Time: $duration_string" # Display duration string (now primarily ms)
if [ $measurement_count -gt 0 ]; then
    printf "  Peak RSS Measured: %'d KB\n" "$peak_rss_kb" # Use printf for locale-aware number formatting
else
    echo "  Peak RSS Measured: Could not measure (process finished too quickly?)"
fi
echo "----------------------------------------"

# Exit with the same code as the monitored command
exit $target_exit_code