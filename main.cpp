#include "worker.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <algorithm>
#include <exception>
#include <stdexcept>


namespace {

    /**
     * @brief Counts the number of lines in a text file by counting newline characters.
     * Includes a check for files that don't end with a newline.
     * @param filename The path to the file.
     * @return The total number of lines, or 0 if the file cannot be opened.
     */
    size_t countTotalLines(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Error: Cannot open file for line counting: " << filename << std::endl;
            return 0;
        }

        size_t lines = std::count(std::istreambuf_iterator<char>(file),
                                  std::istreambuf_iterator<char>(), '\n');

        file.clear();
        file.seekg(0, std::ios::end);
        if (file.tellg() > 0) {
            file.seekg(-1, std::ios::end);
            char last_char;
            if (file.get(last_char)) {
                if (last_char != '\n') {
                    lines++;
                }
            } else {
                if(lines == 0) lines = 1;
            }
        }

        return lines;
    }

    /**
     * @brief Counts the number of lines that appear to be valid Sudoku puzzles.
     * A valid puzzle line is defined as having exactly 81 non-whitespace characters.
     * @param filename The path to the file.
     * @return The number of potential puzzle lines, or 0 if the file cannot be opened.
     */
    size_t countPuzzleLines(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            return 0;
        }
        size_t count = 0;
        std::string line;
        while (std::getline(file, line)) {
            line.erase(std::remove_if(line.begin(), line.end(),
                [](unsigned char c) { return std::isspace(c); }), line.end());
            if (line.size() == 81) {
                count++;
            }
        }
        return count;
    }

    /**
     * @brief Determines the number of threads to use based on hardware and heuristics.
     * Uses command-line argument if provided and valid, otherwise detects hardware
     * concurrency and applies a heuristic (defaults to physical cores for hyperthreaded CPUs).
     * Ensures at least 1 thread and no more threads than total lines.
     *
     * @param cmdLineArg The thread count string from the command line (can be empty or invalid).
     * @param totalLines The total number of lines to process.
     * @return The calculated number of threads to use.
     */
    unsigned determineThreadCount(const std::string& cmdLineArg, size_t totalLines) {
        unsigned numThreads = 0;
        if (!cmdLineArg.empty()) {
            try {
                int nt = std::stoi(cmdLineArg);
                if (nt > 0) {
                    numThreads = static_cast<unsigned>(nt);
                     std::cout << "Using thread count from command line: " << numThreads << std::endl;
                } else {
                    std::cerr << "Warning: Invalid thread count argument '" << cmdLineArg << "' (must be > 0). Using auto-detect.\n";
                }
            } catch (const std::invalid_argument& ) {
                std::cerr << "Warning: Invalid thread count argument '" << cmdLineArg << "'. Using auto-detect.\n";
            } catch (const std::out_of_range& ) {
                std::cerr << "Warning: Thread count argument '" << cmdLineArg << "' out of range. Using auto-detect.\n";
            }
        }

        if (numThreads == 0) {
            unsigned hardware_threads = std::thread::hardware_concurrency();
            if (hardware_threads > 0) {
                if (hardware_threads > 4) {
                    numThreads = std::max(1u, hardware_threads / 2);
                    std::cout << "Auto-detecting threads (estimated physical cores): " << numThreads << std::endl;
                } else {
                    numThreads = hardware_threads;
                    std::cout << "Auto-detecting threads (logical cores): " << numThreads << std::endl;
                }
            } else {
                 std::cout << "Warning: Could not detect hardware concurrency. Defaulting to 1 thread." << std::endl;
                 numThreads = 1;
            }
        }

        unsigned maxPossibleThreads = (totalLines == 0) ? 1 : static_cast<unsigned>(totalLines);
        numThreads = std::min(numThreads, maxPossibleThreads);
        numThreads = std::max(1u, numThreads);

        return numThreads;
    }

}


int main(int argc, char* argv[]) {
    try {
        std::string inputFilename = "input.txt";
        std::string outputFilename = "output.txt";
        std::string threadsArg;

        if (argc > 1) inputFilename = argv[1];
        if (argc > 2) outputFilename = argv[2];
        if (argc > 3) threadsArg = argv[3];

        std::cout << "Input file: " << inputFilename << std::endl;
        std::cout << "Output file: " << outputFilename << std::endl;

        size_t puzzleCount = countPuzzleLines(inputFilename);

        if (puzzleCount == 0 && countTotalLines(inputFilename) == 0) {
             std::cout << "Input file is empty or contains no potential puzzles. Exiting." << std::endl;
             return 0;
        }

        size_t totalLines = countTotalLines(inputFilename);
        if (totalLines == 0 && puzzleCount > 0) { // Inconsistency check
             std::cerr << "Warning: Puzzle lines counted, but total lines seem zero. Check file read permissions/state." << std::endl;
             totalLines = puzzleCount;
        }
        std::cout << "Total lines in file: " << totalLines << std::endl;


        unsigned numThreads = determineThreadCount(threadsArg, totalLines);
        std::cout << "Using " << numThreads << " threads for processing." << std::endl;

        std::vector<std::thread> workers;
        std::ofstream outputFile(outputFilename);
        if (!outputFile) {
            std::cerr << "Error: Cannot open output file: " << outputFilename << std::endl;
            return 1;
        }
        std::mutex outputMutex;
        std::atomic<size_t> solvedCounter(0);
        std::atomic<size_t> processedCounter(0);

        size_t startLine = 0;
        size_t linesPerThread = (totalLines + numThreads - 1) / numThreads;

        workers.reserve(numThreads);

        for (unsigned i = 0; i < numThreads; ++i) {
            size_t endLine = std::min(startLine + linesPerThread, totalLines);
            if (startLine < endLine) {
                workers.emplace_back(SudokuApp::solverWorker,
                                     i,
                                     std::cref(inputFilename),
                                     std::ref(outputFile),
                                     std::ref(outputMutex),
                                     startLine,
                                     endLine,
                                     std::ref(solvedCounter),
                                     std::ref(processedCounter));
            } else {
                std::cout << "Skipping thread creation for empty range starting at line " << startLine << std::endl;
            }
            startLine = endLine;
            if (startLine >= totalLines) break;
        }

        for (auto& t : workers) {
            if (t.joinable()) {
                 t.join();
            }
        }

        outputFile.flush();
        outputFile.close();

    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 2;
    } catch (...) {
         std::cerr << "An unknown error occurred." << std::endl;
         return 3;
    }

    return 0;
}