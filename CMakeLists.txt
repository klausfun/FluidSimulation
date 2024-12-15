cmake_minimum_required(VERSION 3.22)
project(FluidSimulation)

set(CMAKE_CXX_STANDARD 23)

ADD_COMPILE_OPTIONS("-DTYPES=FLOAT,DOUBLE,FIXED(32, 16),FIXED(64, 8),FAST_FIXED(25, 11)")
ADD_COMPILE_OPTIONS("-DSIZES=S(36, 84),S(777,5)")

add_executable(FluidSimulation main.cpp)
