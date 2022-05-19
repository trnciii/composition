cmake_minimum_required(VERSION 3.12)
project(composition CXX)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/composition/core)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/composition/core)


add_subdirectory(${PROJECT_SOURCE_DIR}/src/glm)
include_directories(${PROJECT_SOURCE_DIR}/src/glm)

find_package(OpenMP REQUIRED)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")


add_subdirectory(src/interface) # python interface
add_subdirectory(src/main) # c++ only implementation
