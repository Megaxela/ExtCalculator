cmake_minimum_required(VERSION 3.8)
project(ExtCalculatorBenchmark)

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/benchmark/CMakeLists.txt)
    add_subdirectory(benchmark)
endif()

set(CMAKE_CXX_FLAGS "-O3")

set(CMAKE_CXX_STANDARD 17)

add_executable(ExtCalculatorBenchmark
    main.cpp
)

target_link_libraries(ExtCalculatorBenchmark
    ExtCalculator
    gtest
    benchmark
)