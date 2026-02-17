cmake_minimum_required(VERSION 3.17)
project(example)
add_compile_options("-std=c++11")
find_package(Midas REQUIRED PATHS $ENV{MIDASSYS})
add_executable(fesimdaq fesimdaq.cxx)
target_link_libraries(fesimdaq midas::mfe)




