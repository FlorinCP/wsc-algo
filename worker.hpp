#ifndef SUDOKU_APP_WORKER_HPP
#define SUDOKU_APP_WORKER_HPP

#include <string>   // For std::string argument
#include <cstddef>  // For size_t argument
#include <atomic>   // For std::atomic arguments used by reference
// <fstream> and <mutex> are no longer needed in the signature

namespace SudokuApp {

    /**
     * @brief Processes a designated range of lines from an input file and writes solved puzzles
     *        sequentially to a dedicated temporary output file.
     *
     * Reads puzzle strings from the input file within the specified line range [startLine, endLine),
     * attempts to solve each valid puzzle using a thread-local SudokuSolver instance,
     * and writes *only* the successfully solved puzzle grids to a temporary file specific
     * to this worker. Updates atomic counters for tracking progress.
     *
     * @param workerId An identifier for the worker thread, used to generate the temporary filename.
     * @param inputFilename The path to the input file containing Sudoku puzzles.
     * @param tempFilePrefix A prefix used to construct the temporary output filename (e.g., "output_part_").
     * @param startLine The 0-based starting line index (inclusive) for this worker.
     * @param endLine The 0-based ending line index (exclusive) for this worker.
     * @param solvedCounter An atomic counter to increment upon successfully solving a puzzle.
     * @param processedCounter An atomic counter to increment for each line processed (or attempted).
     * @param errorFlag An atomic flag to indicate if any worker encountered a critical error (like file open failure).
     */
    void solverWorker(
        size_t workerId,
        const std::string& inputFilename,
        const std::string& tempFilePrefix, // New: Prefix for temp file name
        size_t startLine,
        size_t endLine,
        std::atomic<size_t>& solvedCounter,
        std::atomic<size_t>& processedCounter,
        std::atomic<bool>& errorFlag); // New: Flag to signal errors upwards

} // namespace SudokuApp

#endif // SUDOKU_APP_WORKER_HPP