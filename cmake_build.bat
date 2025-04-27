@REM @echo off
mkdir build
cmake . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_PGO_MSVC=ON -DPGO_STAGE=Instrument
cmake --build build --config Release
call build\Release\sudoku_solver.exe
cmake . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_PGO_MSVC=ON -DPGO_STAGE=Optimize
cmake --build build --config Release