#ifndef SUDOKU_APP_SUDOKU_SOLVER_HPP
#define SUDOKU_APP_SUDOKU_SOLVER_HPP

#include <cstdint>
#include <cstddef>

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

    private:
        alignas(64) uint16_t rows[9] = {0};
        alignas(64) uint16_t cols[9] = {0};
        alignas(64) uint16_t boxes[9] = {0};
        char grid[81];
        uint8_t emptyCells[81];
        uint8_t position[81];
        int emptyCount = 0;

        inline bool canPlace(int cell, int num) const;

        void place(int cell, int num);

        void remove(int cell, int num);

        int findMRV() const;

        bool solveInternal();
    };


    inline bool SudokuSolver::canPlace(int cell, int num) const {
        struct CellInfo { uint8_t row, col, box; };
        static constexpr CellInfo preCell_internal_lookup[81] = {
            {0,0,0}, {0,1,0}, {0,2,0}, {0,3,1}, {0,4,1}, {0,5,1}, {0,6,2}, {0,7,2}, {0,8,2},
            {1,0,0}, {1,1,0}, {1,2,0}, {1,3,1}, {1,4,1}, {1,5,1}, {1,6,2}, {1,7,2}, {1,8,2},
            {2,0,0}, {2,1,0}, {2,2,0}, {2,3,1}, {2,4,1}, {2,5,1}, {2,6,2}, {2,7,2}, {2,8,2},
            {3,0,3}, {3,1,3}, {3,2,3}, {3,3,4}, {3,4,4}, {3,5,4}, {3,6,5}, {3,7,5}, {3,8,5},
            {4,0,3}, {4,1,3}, {4,2,3}, {4,3,4}, {4,4,4}, {4,5,4}, {4,6,5}, {4,7,5}, {4,8,5},
            {5,0,3}, {5,1,3}, {5,2,3}, {5,3,4}, {5,4,4}, {5,5,4}, {5,6,5}, {5,7,5}, {5,8,5},
            {6,0,6}, {6,1,6}, {6,2,6}, {6,3,7}, {6,4,7}, {6,5,7}, {6,6,8}, {6,7,8}, {6,8,8},
            {7,0,6}, {7,1,6}, {7,2,6}, {7,3,7}, {7,4,7}, {7,5,7}, {7,6,8}, {7,7,8}, {7,8,8},
            {8,0,6}, {8,1,6}, {8,2,6}, {8,3,7}, {8,4,7}, {8,5,7}, {8,6,8}, {8,7,8}, {8,8,8}
        };


        const uint16_t mask = 1 << num;
        if (cell < 0 || cell >= 81) return false;
        const auto& info = preCell_internal_lookup[cell];
        return !((rows[info.row] | cols[info.col] | boxes[info.box]) & mask);
    }


}

#endif