cmake_minimum_required(VERSION 3.8)
project(ExtCalculatorTests)

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/googletest/CMakeLists.txt)
    add_subdirectory(googletest)
endif()

set(CMAKE_CXX_STANDARD 17)

set(TESTS
        )

add_executable(ExtCalculatorTests
    ${TESTS}
    main.cpp
)

target_link_libraries(ExtCalculatorTests
    ExtCalculator
    gtest
)