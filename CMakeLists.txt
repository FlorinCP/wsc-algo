cmake_minimum_required(VERSION 3.15)

project(sudoku_solver LANGUAGES CXX)

option(ENABLE_AVX2 "Enable AVX2 instructions (Requires compatible CPU, MSVC only)" ON)
if(ENABLE_AVX2 AND NOT MSVC)
    message(WARNING "ENABLE_AVX2 is ON but the compiler is not MSVC. AVX2 flags will not be added.")
endif()

option(ENABLE_PGO_MSVC "Enable MSVC-specific PGO flags (/GENPROFILE, /LTCG:PGO)" OFF)
if(ENABLE_PGO_MSVC AND NOT MSVC)
    message(WARNING "ENABLE_PGO_MSVC is ON but the compiler is not MSVC. PGO flags will not be added.")
endif()
set(PGO_STAGE "" CACHE STRING "PGO Stage (Instrument or Optimize) - only used if ENABLE_PGO_MSVC is ON")
if(ENABLE_PGO_MSVC AND NOT PGO_STAGE STREQUAL "Instrument" AND NOT PGO_STAGE STREQUAL "Optimize")
    message(WARNING "ENABLE_PGO_MSVC is ON, but PGO_STAGE is not set to 'Instrument' or 'Optimize'. PGO flags may not be applied correctly.")
endif()

set(OUTPUT_NAME sudoku_solver)

set(SOURCE_FILES
    src/main.cpp
    src/sudoku_solver.cpp
    src/worker.cpp
)

add_executable(${OUTPUT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
                            headers
                            )

set_target_properties(${OUTPUT_NAME} PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

target_compile_features(${OUTPUT_NAME} PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(${OUTPUT_NAME} PRIVATE /EHsc)

    set_target_properties(${OUTPUT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

    if(ENABLE_AVX2)
        message(STATUS "Enabling AVX2 support for MSVC.")
        target_compile_options(${OUTPUT_NAME} PRIVATE /arch:AVX2)
    endif()

    target_compile_options(${OUTPUT_NAME} PRIVATE "$<$<CONFIG:Release>:/fp:fast>")

    set_property(TARGET ${OUTPUT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

    if(ENABLE_PGO_MSVC)
        message(STATUS "MSVC PGO flags enabled. Current Stage: ${PGO_STAGE}")
        if(PGO_STAGE STREQUAL "Instrument")
            message(STATUS "Adding /GENPROFILE linker flag for PGO instrumentation.")
            target_link_options(${OUTPUT_NAME} PRIVATE "$<$<CONFIG:Release>:/GENPROFILE>")
            target_link_options(${OUTPUT_NAME} PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
        elseif(PGO_STAGE STREQUAL "Optimize")
            message(STATUS "Adding /LTCG:PGO linker flag to use PGO data.")
            target_link_options(${OUTPUT_NAME} PRIVATE "$<$<CONFIG:Release>:/LTCG:PGO>")
        endif()
        target_link_options(${OUTPUT_NAME} PRIVATE "$<$<CONFIG:Release>:/INCREMENTAL:NO>")
    else()
         target_link_options(${OUTPUT_NAME} PRIVATE "$<$<CONFIG:Release>:/INCREMENTAL:NO>")
    endif()


else()
    set_property(TARGET ${OUTPUT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

    message(STATUS "Non-MSVC compiler detected. Applying standard Release optimizations.")
endif()

message(STATUS "Target executable: ${OUTPUT_NAME}")
message(STATUS "Build types available (use -DCMAKE_BUILD_TYPE=): Debug, Release, RelWithDebInfo, MinSizeRel")
if(MSVC)
    message(STATUS "MSVC specific options:")
    message(STATUS "  AVX2 Enabled: ${ENABLE_AVX2}")
    message(STATUS "  PGO Flags Enabled: ${ENABLE_PGO_MSVC}")
    if(ENABLE_PGO_MSVC)
      message(STATUS "    PGO Stage: ${PGO_STAGE}")
      message(STATUS "    PGO Workflow:")
      message(STATUS "      1. Configure with: cmake -DENABLE_PGO_MSVC=ON -DPGO_STAGE=Instrument <source_dir>")
      message(STATUS "      2. Build Release: cmake --build <build_dir> --config Release")
      message(STATUS "      3. Run instrumented executable with representative input: <build_dir>/Release/${OUTPUT_NAME}.exe")
      message(STATUS "      4. Configure with: cmake -DENABLE_PGO_MSVC=ON -DPGO_STAGE=Optimize <source_dir>")
      message(STATUS "      5. Build Release: cmake --build <build_dir> --config Release (this uses PGO data)")
    endif()
endif()