#ifndef SUDOKU_APP_WORKER_HPP
#define SUDOKU_APP_WORKER_HPP

#include <string>
#include <cstddef>
#include <atomic>

namespace SudokuApp {

    void solverWorker(
        size_t workerId,
        const std::string& inputFilename,
        const std::string& tempFilePrefix,
        size_t startLine,
        size_t endLine,
        std::atomic<size_t>& solvedCounter,
        std::atomic<size_t>& processedCounter,
        std::atomic<bool>& errorFlag);

}

#endif