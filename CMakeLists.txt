cmake_minimum_required(VERSION 3.12)
project(composition CXX)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/composition/core)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/composition/core)

# include libs
set(PYBIND11_PYTHON_VERSION 3.9)
add_subdirectory(${PROJECT_SOURCE_DIR}/ext/pybind11)
include_directories(${PROJECT_SOURCE_DIR}/ext/pybind11)

add_subdirectory(${PROJECT_SOURCE_DIR}/ext/glm)
include_directories(${PROJECT_SOURCE_DIR}/ext/glm)

find_package(OpenMP REQUIRED)

include_directories(ext)
include_directories(${PROJECT_SOURCE_DIR}/src)


# options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp-version=50")
endif()


pybind11_add_module(composition src/interface.cpp src/composition.cpp)
if(APPLE)
	target_link_libraries(composition PRIVATE OpenMP::OpenMP_CXX)
endif()


add_executable(main src/main.cpp src/composition.cpp)
if(APPLE)
	target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)
endif()
