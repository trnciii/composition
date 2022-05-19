include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(ref_pt ref_pt.cpp ../kernel.cpp)
add_executable(ref_ppm ref_ppm.cpp ../kernel.cpp)
add_executable(pt_based pt_based.cpp ../kernel.cpp)
add_executable(ppm_based ppm_based.cpp ../kernel.cpp)
