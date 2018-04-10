cmake_minimum_required(VERSION 3.8)


add_library(tinyexpr STATIC
    tinyexpr/tinyexpr.h
    tinyexpr/tinyexpr.c
)

target_include_directories(tinyexpr PUBLIC
    tinyexpr
)