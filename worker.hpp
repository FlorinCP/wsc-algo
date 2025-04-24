#ifndef SUDOKU_APP_WORKER_HPP
#define SUDOKU_APP_WORKER_HPP

#include <string>
#include <cstddef>
#include <atomic>
#include <iosfwd>
#include <fstream>
#include <mutex>

namespace SudokuApp {

    /**
     * @brief Processes a designated range of lines from a Sudoku puzzle input file.
     *
     * Reads puzzle strings from the input file within the specified line range [startLine, endLine),
     * attempts to solve each valid puzzle using a thread-local SudokuSolver instance,
     * and writes the results (or error messages) to a shared output file stream in batches.
     * Uses a mutex to synchronize access to the shared output file. Updates atomic counters
     * for tracking progress. Designed to be run concurrently by multiple threads.
     *
     * @param workerId An identifier for the worker thread (primarily for logging/debugging).
     * @param inputFilename The path to the input file containing Sudoku puzzles.
     * @param outputFile A reference to the opened output file stream for writing results.
     * @param outputMutex A reference to the mutex protecting the outputFile.
     * @param startLine The 0-based starting line index (inclusive) for this worker.
     * @param endLine The 0-based ending line index (exclusive) for this worker.
     * @param solvedCounter An atomic counter to increment upon successfully solving a puzzle.
     * @param processedCounter An atomic counter to increment for each line processed (or attempted).
     */
    void solverWorker(
        size_t workerId,
        const std::string& inputFilename,
        std::ofstream& outputFile,
        std::mutex& outputMutex,
        size_t startLine,
        size_t endLine,
        std::atomic<size_t>& solvedCounter,
        std::atomic<size_t>& processedCounter);

}

#endif