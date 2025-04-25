#include "worker.hpp"
#include "sudoku_solver.hpp"

#include <fstream>   // Still needed for local ifstream/ofstream
#include <string>
#include <algorithm>
#include <atomic>
#include <iostream>  // For std::cerr
#include <vector>    // Included for completeness, good practice

namespace SudokuApp {

void solverWorker(
    size_t workerId,
    const std::string& inputFilename,
    const std::string& tempFilePrefix, // Use the prefix
    size_t startLine,
    size_t endLine,
    std::atomic<size_t>& solvedCounter,
    std::atomic<size_t>& processedCounter,
    std::atomic<bool>& errorFlag) // Receive the error flag
{
    thread_local SudokuSolver solver;

    // --- Construct Temporary Filename ---
    // Example: "output_part_0.tmp", "output_part_1.tmp", etc.
    std::string tempOutputFilename = tempFilePrefix + std::to_string(workerId) + ".tmp";

    // --- Open Local Files ---
    std::ifstream inputFile(inputFilename);
    if (!inputFile) {
        std::cerr << "Worker " << workerId << " Error: Cannot open input file: " << inputFilename << std::endl;
        errorFlag.store(true, std::memory_order_relaxed); // Signal error
        return;
    }

    // Open the temporary file for this worker *only*
    std::ofstream tempOutputFile(tempOutputFilename, std::ios::binary);
    if (!tempOutputFile) {
        std::cerr << "Worker " << workerId << " Error: Cannot open temporary output file: " << tempOutputFilename << std::endl;
        errorFlag.store(true, std::memory_order_relaxed); // Signal error
        inputFile.close(); // Close input file before returning
        return;
    }
    // --- End Open Local Files ---


    std::string line;
    size_t currentLine = 0;

    // --- Output Batching ---
    std::string outputBuffer;
    constexpr size_t BATCH_SIZE = 150; // Keep batching for temp file efficiency
    outputBuffer.reserve(BATCH_SIZE * 81);
    size_t resultsInBatch = 0;
    // --- End Output Batching ---

    // Skip lines
    while (currentLine < startLine && std::getline(inputFile, line)) {
        currentLine++;
    }

    // Process assigned lines
    while (currentLine < endLine && std::getline(inputFile, line)) {
        processedCounter.fetch_add(1, std::memory_order_relaxed);
        line.erase(std::remove_if(line.begin(), line.end(),
                   [](unsigned char c){ return c == '\r' || c == '\n' || std::isspace(c); }), line.end());

        bool was_solved = false;

        if (line.size() == 81) {
            if (solver.initialize(line.data(), line.size())) {
                if (solver.solve()) {
                    was_solved = true;
                }
            }
        }

        // Buffer only if solved
        if (was_solved) {
            outputBuffer.append(solver.getSolution(), 81);
            outputBuffer.push_back('\n');
            resultsInBatch++;
            solvedCounter.fetch_add(1, std::memory_order_relaxed);

            // Write batch to *temporary* file when full (no mutex needed)
            if (resultsInBatch >= BATCH_SIZE) {
                tempOutputFile.write(outputBuffer.data(), outputBuffer.size());
                if (!tempOutputFile) { // Check for write errors
                    std::cerr << "Worker " << workerId << " Error: Failed writing to temp file: " << tempOutputFilename << std::endl;
                    errorFlag.store(true, std::memory_order_relaxed);
                    // Optionally stop processing early on write error
                    // inputFile.close(); tempOutputFile.close(); return;
                }
                outputBuffer.clear();
                resultsInBatch = 0;
            }
        }
        // Check for stream errors periodically (optional but good practice)
        if(inputFile.bad()){
             std::cerr << "Worker " << workerId << " Error: Input file stream error." << std::endl;
             errorFlag.store(true, std::memory_order_relaxed);
             break; // Stop processing this file part
        }

        currentLine++;
    } // End of line processing loop


    // Write any remaining results in the buffer to the temporary file
    if (!outputBuffer.empty()) {
         tempOutputFile.write(outputBuffer.data(), outputBuffer.size());
         if (!tempOutputFile) { // Check for write errors
             std::cerr << "Worker " << workerId << " Error: Failed writing final batch to temp file: " << tempOutputFilename << std::endl;
             errorFlag.store(true, std::memory_order_relaxed);
         }
    }

    // tempOutputFile is closed automatically by RAII when function exits
    // inputFile is closed automatically by RAII when function exits

} // End of solverWorker function

} // namespace SudokuApp