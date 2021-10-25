cmake_minimum_required(VERSION 3.16)
project(ak)

set(CMAKE_CXX_STANDARD 14)

find_package(MPI REQUIRED)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

add_executable(generate generate.cpp)
target_link_libraries(generate ${MPI_CXX_LIBRARIES})

add_executable(sort sort.cpp)
target_link_libraries(sort ${MPI_CXX_LIBRARIES})

add_executable(verify verify.cpp)
target_link_libraries(verify ${MPI_CXX_LIBRARIES})