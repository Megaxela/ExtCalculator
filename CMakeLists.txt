cmake_minimum_required(VERSION 3.8)
project(ExtCalculator)

set(CMAKE_CXX_STANDARD 17)

option(CALC_BUILD_TESTS "Build tests" Off)
option(CALC_BUILD_BENCH "Build benchmark" Off)

#set(CMAKE_CXX_FLAGS "-O3")

if (${CALC_BUILD_TESTS} OR ${CALC_BUILD_BENCH})
    add_subdirectory(tests)
endif()

if (${CALC_BUILD_BENCH})
    add_subdirectory(benchmark)
endif()

set(CMAKE_CXX_FLAGS "-ansi \
                     -pedantic \
                     -Wabi \
                     -Wall \
                     -Wextra \
                     -Weffc++ \
                     -Woverloaded-virtual \
                     -Winit-self \
                     -Wunreachable-code \
                     -Wno-error")

add_subdirectory(cli)

set(INCLUDE_FILES
    include/Calculator.hpp
    include/ParsingException.hpp
    include/StatementException.hpp
    include/CalculationException.hpp
)

set(SOURCE_FILES
    src/Calculator.cpp
)

add_library(ExtCalculator STATIC
    ${INCLUDE_FILES}
    ${SOURCE_FILES}
)

target_include_directories(ExtCalculator PUBLIC
    include
)