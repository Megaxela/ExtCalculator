cmake_minimum_required(VERSION 3.8)
project(ExtCalculatorCLI)

set(CMAKE_CXX_STANDARD 17)

set(INCLUDE_FILES

)

set(SOURCE_FILES
    main.cpp
)

add_executable(ExtCalculatorCLI
    ${INCLUDE_FILES}
    ${SOURCE_FILES}
    main.cpp
)

target_link_libraries(ExtCalculatorCLI
    ExtCalculator
)