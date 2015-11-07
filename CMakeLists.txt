cmake_minimum_required(VERSION 2.8)
project(issen)

include_directories("${PROJECT_SOURCE_DIR}/include")
set(CMAKE_CXX_FLAGS "-std=c++14 -O3 -flto -march=native -mtune=native -Wall -Wextra")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
add_subdirectory(src)
