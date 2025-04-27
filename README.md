## Running on Windows

To build the project on Windows, open the **Developer Command Prompt for Visual Studio** and run:
The CMake is not tested, it might work or not, I've developed everything on MacOS...
The PGO might not work on Windows as expected, with "custom" evaluator on Mac I got avg. of 1050ms and 3800KB.

```
florinpeana@Florins-MacBook-Pro algo % ./memory.sh ./sudoku_solver                                   
[Monitor] Starting command: ./sudoku_solver
[Monitor] Target PID: 71167
[Monitor] Monitoring RSS (Peak KB)...
Auto-detecting threads: 6
Concatenating temporary files into output.txt...
[Monitor] Monitoring stopped.
----------------------------------------
Command Execution Finished
  Exit Code: 0
  Execution Time: 1274 ms
  Peak RSS Measured: 3840 KB
----------------------------------------
florinpeana@Florins-MacBook-Pro algo % ./memory.sh ./sudoku_solver
[Monitor] Starting command: ./sudoku_solver
[Monitor] Target PID: 71202
[Monitor] Monitoring RSS (Peak KB)...
Auto-detecting threads: 6
Concatenating temporary files into output.txt...
[Monitor] Monitoring stopped.
----------------------------------------
Command Execution Finished
  Exit Code: 0
  Execution Time: 1057 ms
  Peak RSS Measured: 3812 KB
----------------------------------------
florinpeana@Florins-MacBook-Pro algo % ./memory.sh ./sudoku_solver
[Monitor] Starting command: ./sudoku_solver
[Monitor] Target PID: 71228
[Monitor] Monitoring RSS (Peak KB)...
Auto-detecting threads: 6
Concatenating temporary files into output.txt...
[Monitor] Monitoring stopped.
----------------------------------------
Command Execution Finished
  Exit Code: 0
  Execution Time: 1070 ms
  Peak RSS Measured: 3840 KB
----------------------------------------
florinpeana@Florins-MacBook-Pro algo % ./memory.sh ./sudoku_solver
[Monitor] Starting command: ./sudoku_solver
[Monitor] Target PID: 71257
[Monitor] Monitoring RSS (Peak KB)...
Auto-detecting threads: 6
Concatenating temporary files into output.txt...
[Monitor] Monitoring stopped.
----------------------------------------
Command Execution Finished
  Exit Code: 0
  Execution Time: 1061 ms
  Peak RSS Measured: 3808 KB
----------------------------------------
florinpeana@Florins-MacBook-Pro algo % ./memory.sh ./sudoku_solver
[Monitor] Starting command: ./sudoku_solver
[Monitor] Target PID: 71283
[Monitor] Monitoring RSS (Peak KB)...
Auto-detecting threads: 6
Concatenating temporary files into output.txt...
[Monitor] Monitoring stopped.
----------------------------------------
Command Execution Finished
  Exit Code: 0
  Execution Time: 1066 ms
  Peak RSS Measured: 3820 KB
----------------------------------------
florinpeana@Florins-MacBook-Pro algo % ./memory.sh ./sudoku_solver
[Monitor] Starting command: ./sudoku_solver
[Monitor] Target PID: 71309
[Monitor] Monitoring RSS (Peak KB)...
Auto-detecting threads: 6
Concatenating temporary files into output.txt...
[Monitor] Monitoring stopped.
----------------------------------------
Command Execution Finished
  Exit Code: 0
  Execution Time: 1064 ms
  Peak RSS Measured: 3812 KB
----------------------------------------
florinpeana@Florins-MacBook-Pro algo % ./memory.sh ./sudoku_solver
[Monitor] Starting command: ./sudoku_solver
[Monitor] Target PID: 71337
[Monitor] Monitoring RSS (Peak KB)...
Auto-detecting threads: 6
Concatenating temporary files into output.txt...
[Monitor] Monitoring stopped.
----------------------------------------
Command Execution Finished
  Exit Code: 0
  Execution Time: 1066 ms
  Peak RSS Measured: 3816 KB
----------------------------------------
florinpeana@Florins-MacBook-Pro algo % 

```

```bash
    cl_build.bat
```

MacOS

```bash
    build.sh
```
