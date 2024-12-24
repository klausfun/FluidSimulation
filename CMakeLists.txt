cmake_minimum_required(VERSION 3.22.1)

project(FluidSimulation)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS -O3)
add_definitions("-DTYPES=FLOAT,DOUBLE,FIXED(32,16),FAST_FIXED(48,16)")
add_definitions("-DSIZES=S(24,84),S(50,50)")

include_directories("src/")

add_executable(main main.cpp)
