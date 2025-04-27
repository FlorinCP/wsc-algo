#include "worker.hpp"
#include "sudoku_solver.hpp"

#include <fstream>
#include <string>
#include <algorithm>
#include <atomic>
#include <iostream>
#include <vector>

namespace SudokuApp {

void solverWorker(
    size_t workerId,
    const std::string& inputFilename,
    const std::string& tempFilePrefix,
    size_t startLine,
    size_t endLine,
    std::atomic<size_t>& solvedCounter,
    std::atomic<size_t>& processedCounter,
    std::atomic<bool>& errorFlag)
{
    thread_local SudokuSolver solver;

    std::string tempOutputFilename = tempFilePrefix + std::to_string(workerId) + ".tmp";

    std::ifstream inputFile(inputFilename);
    if (!inputFile) {
        std::cerr << "Worker " << workerId << " Error: Cannot open input file: " << inputFilename << std::endl;
        errorFlag.store(true, std::memory_order_relaxed);
        return;
    }

    std::ofstream tempOutputFile(tempOutputFilename, std::ios::binary);
    if (!tempOutputFile) {
        std::cerr << "Worker " << workerId << " Error: Cannot open temporary output file: " << tempOutputFilename << std::endl;
        errorFlag.store(true, std::memory_order_relaxed);
        inputFile.close();
        return;
    }

    std::string line;
    size_t currentLine = 0;

    std::string outputBuffer;
    constexpr size_t BATCH_SIZE = 150;
    outputBuffer.reserve(BATCH_SIZE * 81);
    size_t resultsInBatch = 0;

    while (currentLine < startLine && std::getline(inputFile, line)) {
        currentLine++;
    }

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

        if (was_solved) {
            outputBuffer.append(solver.getSolution(), 81);
            outputBuffer.push_back('\n');
            resultsInBatch++;
            solvedCounter.fetch_add(1, std::memory_order_relaxed);

            if (resultsInBatch >= BATCH_SIZE) {
                tempOutputFile.write(outputBuffer.data(), outputBuffer.size());
                if (!tempOutputFile) {
                    std::cerr << "Worker " << workerId << " Error: Failed writing to temp file: " << tempOutputFilename << std::endl;
                    errorFlag.store(true, std::memory_order_relaxed);
                }
                outputBuffer.clear();
                resultsInBatch = 0;
            }
        }
        if(inputFile.bad()){
             std::cerr << "Worker " << workerId << " Error: Input file stream error." << std::endl;
             errorFlag.store(true, std::memory_order_relaxed);
             break;
        }

        currentLine++;
    }


    if (!outputBuffer.empty()) {
         tempOutputFile.write(outputBuffer.data(), outputBuffer.size());
         if (!tempOutputFile) {
             std::cerr << "Worker " << workerId << " Error: Failed writing final batch to temp file: " << tempOutputFilename << std::endl;
             errorFlag.store(true, std::memory_order_relaxed);
         }
    }


}

}