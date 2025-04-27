#include "sudoku_solver.hpp"
#include <cstring>           // For memset, memcpy
#include <cstdint>           // For uint types (used implicitly via header, good practice)


#if defined(_MSC_VER)
    #include <intrin.h>

    inline int portable_popcount(unsigned int n) {
        return static_cast<int>(__popcnt(n));
    }
    inline int portable_popcount(uint16_t n) {
        return static_cast<int>(__popcnt(static_cast<unsigned int>(n)));
    }

    inline int portable_ctz(unsigned int n) {
        if (n == 0) return 32;
        unsigned long index;
        _BitScanForward(&index, n);
        return static_cast<int>(index);
    }
    inline int portable_ctz(uint16_t n) {
         if (n == 0) return 16;
         return portable_ctz(static_cast<unsigned int>(n));
    }

#elif defined(__GNUC__) || defined(__clang__)
    inline int portable_popcount(unsigned int n) {
        return __builtin_popcount(n);
    }
    inline int portable_popcount(uint16_t n) {
        return __builtin_popcount(static_cast<unsigned int>(n));
    }

    inline int portable_ctz(unsigned int n) {
        return __builtin_ctz(n);
    }
    inline int portable_ctz(uint16_t n) {
        return __builtin_ctz(static_cast<unsigned int>(n));
    }

#endif


namespace SudokuApp {

    struct CellInfo_Internal {
        uint8_t row, col, box;
    };
    static constexpr CellInfo_Internal preCell_lookup[81] = {
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


    bool SudokuSolver::initialize(const char* puzzle_data, size_t len) {
        if (len != 81 || puzzle_data == nullptr) return false;
        emptyCount = 0;
        memset(rows, 0, sizeof(rows));
        memset(cols, 0, sizeof(cols));
        memset(boxes, 0, sizeof(boxes));
        memcpy(grid, puzzle_data, 81);
        memset(position, 81, sizeof(position));
        bool initial_valid = true;
        for (int i = 0; i < 81; ++i) {
            int num = 0;
            if (grid[i] >= '1' && grid[i] <= '9') { num = grid[i] - '0'; }
            else { grid[i] = '0'; }
            if (num != 0) {
                const uint16_t mask = 1 << num;
                const auto& info = preCell_lookup[i];
                if ((rows[info.row] & mask) || (cols[info.col] & mask) || (boxes[info.box] & mask)) {
                    initial_valid = false; // Keep checking or return false;
                }
                rows[info.row] |= mask; cols[info.col] |= mask; boxes[info.box] |= mask;
            } else {
                if (emptyCount < 81) {
                    emptyCells[emptyCount] = static_cast<uint8_t>(i);
                    position[i] = static_cast<uint8_t>(emptyCount);
                    emptyCount++;
                } else { return false; }
            }
        }
        return initial_valid;
    }


    bool SudokuSolver::solve() {
        for(int i=0; i<9; ++i) {
            if(portable_popcount(rows[i] & 0x3FEu) != portable_popcount(rows[i])) return false;
            if(portable_popcount(cols[i] & 0x3FEu) != portable_popcount(cols[i])) return false;
            if(portable_popcount(boxes[i] & 0x3FEu) != portable_popcount(boxes[i])) return false;
        }
        return solveInternal();
    }


    const char* SudokuSolver::getSolution() const {
        return grid;
    }


    void SudokuSolver::place(int cell, int num) {
        if (cell < 0 || cell >= 81 || num < 1 || num > 9) return;
        const auto& info = preCell_lookup[cell];
        const uint16_t mask = 1 << num;
        rows[info.row] |= mask; cols[info.col] |= mask; boxes[info.box] |= mask;
        grid[cell] = '0' + num;
        if (emptyCount > 0) {
            const int pos = position[cell];
            if (pos < emptyCount && pos >= 0) {
                const uint8_t last_cell_index = emptyCells[emptyCount - 1];
                if(last_cell_index < 81) {
                    emptyCells[pos] = last_cell_index;
                    position[last_cell_index] = pos;
                }
                position[cell] = 81;
            }
            emptyCount--;
        }
    }


    void SudokuSolver::remove(int cell, int num) {
        if (cell < 0 || cell >= 81 || num < 1 || num > 9) return;
        const auto& info = preCell_lookup[cell];
        const uint16_t mask = ~(1 << num);
        rows[info.row] &= mask; cols[info.col] &= mask; boxes[info.box] &= mask;
        grid[cell] = '0';
        if (emptyCount < 81) {
            position[cell] = emptyCount;
            emptyCells[emptyCount] = static_cast<uint8_t>(cell);
            emptyCount++;
        }
    }


    int SudokuSolver::findMRV() const {
        int minCount = 10;
        int bestCell = -1;
        for (int i = 0; i < emptyCount; ++i) {
            const int cell = emptyCells[i];
            if (cell < 0 || cell >= 81) continue;
            const auto& info = preCell_lookup[cell];
            const uint16_t used = rows[info.row] | cols[info.col] | boxes[info.box];
            // Use the portable popcount wrapper
            const int count = portable_popcount(static_cast<uint16_t>(~used & 0x3FEu));

            if (count == 0) return -1;
            if (count < minCount) {
                minCount = count;
                bestCell = cell;
                if (minCount == 1) break;
            }
        }
         if (bestCell == -1 && emptyCount > 0) {
             bestCell = emptyCells[0];
         }
        return bestCell;
    }


    bool SudokuSolver::solveInternal() {
        if (emptyCount == 0) return true;
        const int cell = findMRV();
        if (cell == -1) return false;
        if (cell < 0 || cell >= 81) return false;

        const auto& info = preCell_lookup[cell];
        const uint16_t used = rows[info.row] | cols[info.col] | boxes[info.box];
        uint16_t possible = ~used & 0x3FEu;

        while (possible) {
            const int num = portable_ctz(possible);

            if (num < 1 || num > 9) {
                 possible &= (possible - 1);
                 continue;
            }

            place(cell, num);
            if (solveInternal()) { return true; }
            remove(cell, num);
            possible &= ~(1 << num);
        }
        return false;
    }

}