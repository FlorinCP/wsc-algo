#ifndef SUDOKU_APP_SUDOKU_SOLVER_HPP // Updated header guard convention
#define SUDOKU_APP_SUDOKU_SOLVER_HPP

#include <cstdint> // For uint8_t, uint16_t
#include <cstddef> // For size_t

// Forward declare CellInfo if needed internally by inline methods,
// but it's better if inline methods don't depend on internal structs.
// struct CellInfo; // No longer needed here if preCell lookup is internal

namespace SudokuApp {

    /**
     * @brief Solves Sudoku puzzles using a backtracking algorithm with bitmasks and MRV heuristic.
     *
     * This class maintains the state of a single Sudoku grid and provides methods
     * to initialize it with a puzzle string, solve it, and retrieve the solution.
     * It is designed to be efficient for solving standard 9x9 Sudoku puzzles.
     * Note: This class is intended for single-threaded use per instance. For multi-threading,
     * use separate instances (e.g., via `thread_local`).
     */
    class SudokuSolver {
    public:
        /**
         * @brief Initializes the solver state with a puzzle string.
         *
         * Resets internal state and populates the grid and bitmasks based on the input.
         * The input string must represent an 81-character grid, using '1'-'9' for filled cells
         * and '0' or '.' (or any other character) for empty cells.
         * Basic validation (no immediate conflicts) is performed during initialization.
         *
         * @param puzzle_data A pointer to the character array containing the puzzle string.
         * @param len The length of the puzzle string (must be 81).
         * @return true if the input string has the correct length and the initial puzzle
         *         state has no obvious rule violations (duplicate numbers in row/col/box).
         * @return false otherwise.
         */
        bool initialize(const char* puzzle_data, size_t len);

        /**
         * @brief Attempts to solve the initialized puzzle using backtracking.
         *
         * Assumes `initialize` has been called successfully. Modifies the internal grid state.
         *
         * @return true if a valid solution is found.
         * @return false if the puzzle is unsolvable from the initialized state.
         */
        bool solve();

        /**
         * @brief Gets a pointer to the current state of the Sudoku grid.
         *
         * Returns a pointer to the internal 81-character buffer. If `solve()` returned true,
         * this buffer contains the solution. If `solve()` returned false or was not called,
         * it contains the grid in its current state after initialization or partial solving attempt.
         * The pointer remains valid only for the lifetime of the SudokuSolver object.
         *
         * @return const char* Pointer to the 81-character grid buffer.
         */
        const char* getSolution() const;

        // --- Destructor (Default is sufficient) ---
        // ~SudokuSolver() = default;

        // --- Copy/Move Semantics (Default might be okay, but consider if needed) ---
        // SudokuSolver(const SudokuSolver&) = default;
        // SudokuSolver& operator=(const SudokuSolver&) = default;
        // SudokuSolver(SudokuSolver&&) = default;
        // SudokuSolver& operator=(SudokuSolver&&) = default;


    private:
        // --- Member Variables ---
        // Keep internal representation hidden. Precise types are implementation details.
        alignas(64) uint16_t rows[9] = {0};
        alignas(64) uint16_t cols[9] = {0};
        alignas(64) uint16_t boxes[9] = {0};
        char grid[81];
        uint8_t emptyCells[81];
        uint8_t position[81];
        int emptyCount = 0;

        // --- Internal Helper Method Declarations ---
        // Declarations only; implementations belong in the .cpp file unless truly trivial.

        // Checks if 'num' can be placed at 'cell' index. (Keep inline for performance)
        inline bool canPlace(int cell, int num) const;

        // Places 'num' at 'cell', updates state. (Implementation in .cpp)
        void place(int cell, int num);

        // Removes 'num' from 'cell', updates state. (Implementation in .cpp)
        void remove(int cell, int num);

        // Finds the best empty cell to try next (MRV heuristic). (Implementation in .cpp)
        int findMRV() const;

        // The core recursive solving logic. (Implementation in .cpp)
        bool solveInternal();
    }; // class SudokuSolver


    // --- Inline Method Definitions ---
    // Define only truly small, performance-critical helpers inline in the header.
    // `canPlace` is a good candidate. `place` and `remove` update more state
    // and moving them to the .cpp improves header cleanliness and build times.

    inline bool SudokuSolver::canPlace(int cell, int num) const {
        // Implementation note: This relies on preCell being available, which
        // is an internal detail. To be strictly clean, this lookup logic
        // should ideally also move to the .cpp, or preCell needs to be accessible
        // in a hidden way (e.g., static member lookup function).
        // However, for performance, direct lookup via preCell defined in the .cpp
        // is often done, accepting this slight encapsulation breach for speed.
        // Let's assume preCell will be defined in the .cpp file.

        // Forward declaration of the internal lookup array structure is needed
        // if we strictly hide it AND keep this inline.
        // struct CellInfo; // Forward declare
        // extern const CellInfo preCell[81]; // Declare external linkage (complex setup)

        // --- Simpler Approach: Assume preCell is accessible via SudokuSolver.cpp ---
        // This requires linking SudokuSolver.cpp to use this header.
        // (This is the most common practical approach)

        // Need access to preCell, defined in the CPP file.
        // This will work because the *linker* resolves it, but strictly it's impure.
        // A cleaner way requires a helper function. Let's keep it practical for now.
        // --> We NEED preCell definition visible to compile this inline method here.
        // --> Reverting: Move preCell back here for the inline definition to work,
        //     accepting it's exposed but `static constexpr` limits its scope somewhat.

        // --- Reinstating preCell for inline canPlace ---
        struct CellInfo { uint8_t row, col, box; };
        // Using static hides it from other translation units unless explicitly declared extern
        // Using static implies internal linkage.
        static constexpr CellInfo preCell_internal_lookup[81] = {
            {0,0,0}, {0,1,0}, {0,2,0}, /* ... rest of preCell data ... */ {8,8,8}
        };
        // --- End Reinstating ---


        const uint16_t mask = 1 << num;
        if (cell < 0 || cell >= 81) return false; // Bounds check
        // Use the locally defined lookup table
        const auto& info = preCell_internal_lookup[cell];
        return !((rows[info.row] | cols[info.col] | boxes[info.box]) & mask);
    }


} // namespace SudokuApp

#endif // SUDOKU_APP_SUDOKU_SOLVER_HPP