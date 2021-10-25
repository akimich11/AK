cmake_minimum_required(VERSION 3.16)
project(ak)

set(CMAKE_CXX_STANDARD 14)

find_package(MPI REQUIRED)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_executable(main sort.cpp)
target_link_libraries(main ${MPI_CXX_LIBRARIES})