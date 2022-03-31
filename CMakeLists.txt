cmake_minimum_required(VERSION 3.12)
project(composition CXX)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories(${PROJECT_SOURCE_DIR}/src)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/composition/core)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/composition/core)

# include libs
set(PYBIND11_PYTHON_VERSION 3.9)
add_subdirectory(${PROJECT_SOURCE_DIR}/ext/pybind11)

find_package(OpenMP REQUIRED)

find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/ext)


# options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")


# add the python library
if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp-version=50")
	add_library(composition SHARED src/interface.cpp src/composition.cpp)
	set_target_properties(composition PROPERTIES PREFIX "")
	set_target_properties(composition PROPERTIES SUFFIX ".so")
	target_link_libraries(composition PRIVATE OpenMP::OpenMP_CXX)
else()
	pybind11_add_module(composition src/interface.cpp src/composition.cpp)
endif()

# add the cpp only executable
if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp-version=50")
	add_executable(main src/main.cpp src/composition.cpp)
	target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)
else()
	add_executable(main src/main.cpp src/composition.cpp)
endif()