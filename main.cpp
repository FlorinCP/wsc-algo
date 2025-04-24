#include "worker.hpp" // Provides the solverWorker function declaration

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
// #include <mutex> // No longer needed here
#include <atomic>
#include <algorithm>
#include <exception>
#include <iterator>
#include <functional>
#include <stdexcept>
#include <cstdio> // For std::remove (deleting temp files)

namespace { // Anonymous namespace for utility functions

    // countTotalLines, countPuzzleLines, determineThreadCount functions remain the same...
    // (Paste the versions from the previous good response here)
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
            if (file.get(last_char)) { if (last_char != '\n') lines++; }
            else { if(lines == 0) lines = 1; }
        }
        return lines;
    }
    size_t countPuzzleLines(const std::string& filename) {
        std::ifstream file(filename); if (!file) return 0;
        size_t count = 0; std::string line;
        while (std::getline(file, line)) {
            line.erase(std::remove_if(line.begin(), line.end(),
                [](unsigned char c) { return std::isspace(c); }), line.end());
            if (line.size() == 81) count++;
        }
        return count;
    }
    unsigned determineThreadCount(const std::string& cmdLineArg, size_t totalLines) {
        unsigned numThreads = 0;
        if (!cmdLineArg.empty()) {
            try {
                int nt = std::stoi(cmdLineArg);
                if (nt > 0) numThreads = static_cast<unsigned>(nt);
                else std::cerr << "Warning: Invalid thread count '" << cmdLineArg << "'. Auto-detecting.\n";
            } catch (...) { std::cerr << "Warning: Invalid thread count '" << cmdLineArg << "'. Auto-detecting.\n"; }
        }
        if (numThreads == 0) {
            unsigned hardware_threads = std::thread::hardware_concurrency();
            if (hardware_threads > 0) {
                if (hardware_threads > 4) numThreads = std::max(1u, hardware_threads / 2);
                else numThreads = hardware_threads;
                std::cout << "Auto-detecting threads: " << numThreads << std::endl;
            } else { std::cout << "Warning: Cannot detect concurrency. Using 1 thread." << std::endl; numThreads = 1; }
        }
        unsigned maxPossibleThreads = (totalLines == 0) ? 1 : static_cast<unsigned>(totalLines);
        numThreads = std::min(numThreads, maxPossibleThreads);
        numThreads = std::max(1u, numThreads);
        return numThreads;
    }


    /**
     * @brief Concatenates temporary part files into the final output file.
     * Also cleans up the temporary files afterwards.
     * @param finalOutputFilename The name of the final output file.
     * @param tempFilePrefix The prefix used for temporary files.
     * @param numThreads The number of worker threads (and thus part files).
     * @return true on success, false if any file operation fails.
     */
    bool concatenateOutputFiles(const std::string& finalOutputFilename,
                                const std::string& tempFilePrefix,
                                unsigned numThreads)
    {
        std::cout << "Concatenating temporary files into " << finalOutputFilename << "..." << std::endl;
        std::ofstream finalOutputFile(finalOutputFilename);
        if (!finalOutputFile) {
            std::cerr << "Error: Cannot open final output file for writing: " << finalOutputFilename << std::endl;
            return false;
        }

        // Buffer for reading from temp files
        constexpr size_t READ_BUFFER_SIZE = 8192; // 8KB buffer
        std::vector<char> buffer(READ_BUFFER_SIZE);

        bool success = true;

        for (unsigned i = 0; i < numThreads; ++i) {
            std::string tempFilename = tempFilePrefix + std::to_string(i) + ".tmp";
            std::ifstream tempInputFile(tempFilename, std::ios::binary); // Open in binary for efficient copying

            if (!tempInputFile) {
                // It's possible a thread processed an empty range or had an error,
                // so the temp file might not exist. Don't treat this as a fatal error
                // unless we expect *all* threads to always produce output.
                 std::cerr << "Warning: Could not open temporary file: " << tempFilename << ". Skipping." << std::endl;
                // If this *should* always exist, set success = false here.
                continue;
            }

            // Read from temp file and write to final output file
            while (tempInputFile.read(buffer.data(), buffer.size())) {
                finalOutputFile.write(buffer.data(), tempInputFile.gcount());
                if(!finalOutputFile) {
                    std::cerr << "Error: Failed writing to final output file: " << finalOutputFilename << std::endl;
                    success = false; break;
                }
            }
            // Handle the last partial read (if any)
            if (success && tempInputFile.gcount() > 0) {
                finalOutputFile.write(buffer.data(), tempInputFile.gcount());
                 if(!finalOutputFile) {
                    std::cerr << "Error: Failed writing final part to output file: " << finalOutputFilename << std::endl;
                    success = false;
                }
            }
             if(!success) break; // Stop if write failed

            tempInputFile.close(); // Close the temporary file handle

            // Delete the temporary file after successful concatenation
            if (std::remove(tempFilename.c_str()) != 0) {
                 // Report error but don't necessarily fail the whole process
                 std::cerr << "Warning: Could not remove temporary file: " << tempFilename << std::endl;
            }
        }

        finalOutputFile.close(); // Ensure final file is closed

        if(success) {
            std::cout << "Concatenation complete." << std::endl;
        } else {
             std::cerr << "Concatenation failed due to errors." << std::endl;
        }
        return success;
    }


} // namespace


int main(int argc, char* argv[]) {
    try {
        // --- Configuration and Argument Parsing ---
        std::string inputFilename = "input.txt";
        std::string outputFilename = "output.txt";
        std::string threadsArg = "";
        // Define a base name for temporary files
        const std::string tempFilePrefix = outputFilename + "_part_";

        if (argc > 1) inputFilename = argv[1];
        if (argc > 2) outputFilename = argv[2];
        if (argc > 3) threadsArg = argv[3];

        std::cout << "Input file: " << inputFilename << std::endl;
        std::cout << "Output file: " << outputFilename << std::endl;

        // --- File Pre-checks and Counting ---
        size_t puzzleCount = countPuzzleLines(inputFilename);
        std::cout << "Found " << puzzleCount << " potential puzzles." << std::endl;
        size_t totalLines = countTotalLines(inputFilename);
        if (totalLines == 0) {
             std::cout << "Input file is empty or unreadable. Exiting." << std::endl;
             return (puzzleCount > 0) ? 1 : 0; // Error if puzzles expected but no lines read
        }
        std::cout << "Total lines in file: " << totalLines << std::endl;

        // --- Thread Determination ---
        unsigned numThreads = determineThreadCount(threadsArg, totalLines);
        std::cout << "Using " << numThreads << " threads for processing." << std::endl;

        // --- Resource Initialization ---
        std::vector<std::thread> workers;
        // std::ofstream outputFile(outputFilename); // DON'T open final output yet
        // std::mutex outputMutex; // DON'T need mutex anymore
        std::atomic<size_t> solvedCounter(0);
        std::atomic<size_t> processedCounter(0);
        std::atomic<bool> workerErrorFlag(false); // Flag for workers to signal critical errors


        // --- Work Distribution and Thread Creation ---
        size_t startLine = 0;
        size_t linesPerThread = (totalLines + numThreads - 1) / numThreads;
        workers.reserve(numThreads);

        for (unsigned i = 0; i < numThreads; ++i) {
            size_t endLine = std::min(startLine + linesPerThread, totalLines);
            if (startLine < endLine) {
                // Pass tempFilePrefix instead of outputFile and outputMutex
                // Pass the error flag reference
                workers.emplace_back(SudokuApp::solverWorker,
                                     i,
                                     std::cref(inputFilename),
                                     std::cref(tempFilePrefix), // Pass prefix by const ref
                                     startLine,
                                     endLine,
                                     std::ref(solvedCounter),
                                     std::ref(processedCounter),
                                     std::ref(workerErrorFlag)); // Pass error flag by ref
            }
            startLine = endLine;
            if (startLine >= totalLines) break;
        }

        // --- Wait for Threads to Complete ---
        std::cout << "Waiting for " << workers.size() << " worker thread(s) to finish..." << std::endl;
        for (auto& t : workers) {
            if (t.joinable()) {
                 t.join();
            }
        }
        std::cout << "All workers finished." << std::endl;


        // --- Post-Processing: Concatenate Files and Report ---
        bool concatenation_ok = false;
        if (workerErrorFlag.load()) {
             std::cerr << "One or more workers reported an error. Skipping concatenation." << std::endl;
             // Optionally try to clean up any temp files that might exist
             for (unsigned i = 0; i < numThreads; ++i) std::remove((tempFilePrefix + std::to_string(i) + ".tmp").c_str());
        } else {
            // Concatenate temporary files into the final output file
            concatenation_ok = concatenateOutputFiles(outputFilename, tempFilePrefix, numThreads);
        }


        // Print final summary based on counters.
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "Processing Summary:" << std::endl;
        std::cout << "  Lines processed by workers: " << processedCounter.load() << " (Target: " << totalLines << ")" << std::endl;
        std::cout << "  Successfully solved puzzles:  " << solvedCounter.load() << std::endl;
        if(!workerErrorFlag.load() && !concatenation_ok) {
             std::cout << "  WARNING: Concatenation step failed!" << std::endl;
        } else if (workerErrorFlag.load()){
             std::cout << "  WARNING: Worker error occurred during processing!" << std::endl;
        }
        std::cout << "----------------------------------------" << std::endl;


    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 2;
    } catch (...) {
         std::cerr << "An unknown error occurred." << std::endl;
         return 3;
    }

    return 0;
}