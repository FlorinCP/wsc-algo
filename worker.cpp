#include "worker.hpp"
#include "sudoku_solver.hpp"

#include <fstream>   // For std::ifstream, std::ofstream
#include <string>    // For std::string, std::getline, std::to_string
#include <algorithm> // For std::remove_if
#include <mutex>     // For std::lock_guard
#include <atomic>    // Required by function signature
#include <iostream>  // For std::cerr (error reporting for file open failure)

namespace SudokuApp {

void solverWorker(
    size_t workerId,
    const std::string& inputFilename,
    std::ofstream& outputFile,
    std::mutex& outputMutex,
    size_t startLine,
    size_t endLine,
    std::atomic<size_t>& solvedCounter,
    std::atomic<size_t>& processedCounter)
{
    thread_local SudokuSolver solver;

    std::ifstream inputFile(inputFilename);
    if (!inputFile) {
        std::cerr << "Worker " << workerId << " Error: Cannot open input file: " << inputFilename << std::endl;
        return;
    }

    std::string line;
    size_t currentLine = 0;

    // --- Output Batching Configuration & State ---
    std::string outputBuffer;
    constexpr size_t BATCH_SIZE = 150;
    outputBuffer.reserve(BATCH_SIZE * 81); // Buffer now only stores solutions (81 chars)
                                           // No newline needed in reserve calculation if added later
    size_t resultsInBatch = 0;
    // --- End Batching ---

    // Skip lines before the assigned start line
    while (currentLine < startLine && std::getline(inputFile, line)) {
        currentLine++;
    }

    // Process lines within the assigned range [startLine, endLine).
    while (currentLine < endLine && std::getline(inputFile, line)) {
        processedCounter.fetch_add(1, std::memory_order_relaxed);

        line.erase(std::remove_if(line.begin(), line.end(),
                   [](unsigned char c){ return std::isspace(c); }), line.end());

        bool was_solved = false;

        // --- Puzzle Processing Logic ---
        // Only proceed if the line looks like a potential puzzle
        if (line.size() == 81) {
            // Attempt to initialize the solver. If initialization fails (invalid puzzle),
            // we simply do nothing further for this line.
            if (solver.initialize(line.data(), line.size())) {
                // If initialization succeeded, attempt to solve.
                if (solver.solve()) {
                    // Puzzle solved successfully. Mark it.
                    was_solved = true;
                }
                // If !solver.solve() (unsolvable), do nothing.
            }
            // If !solver.initialize() (invalid input), do nothing.
        }
        // If line.size() != 81 (malformed or empty), do nothing.
        // --- End Puzzle Processing Logic ---


        // --- Buffer Management ---
        // Add to the buffer *only if* the puzzle was successfully solved.
        if (was_solved) {
            // Append the solution grid (81 chars)
            outputBuffer.append(solver.getSolution(), 81);
            // Append the newline character AFTER the grid
            outputBuffer.push_back('\n');
            resultsInBatch++;
            solvedCounter.fetch_add(1, std::memory_order_relaxed);

            // Check if the buffer is full and needs to be written.
            if (resultsInBatch >= BATCH_SIZE) {
                std::lock_guard<std::mutex> lock(outputMutex);
                outputFile.write(outputBuffer.data(), outputBuffer.size());
                outputBuffer.clear();
                resultsInBatch = 0;
            }
        }
        // --- End Buffer Management ---

        currentLine++; // Move to the next line index.
    } // End of line processing loop

    // After processing all assigned lines, write any remaining results left in the buffer.
    if (!outputBuffer.empty()) {
        std::lock_guard<std::mutex> lock(outputMutex);
        outputFile.write(outputBuffer.data(), outputBuffer.size());
    }

} // End of solverWorker function

} // namespace SudokuApp