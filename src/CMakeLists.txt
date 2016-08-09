cmake_minimum_required(VERSION 2.8)
add_executable(issen bit_manipulations.cpp gcc_intrinsics.cpp hand.cpp line.cpp state.cpp utils.cpp main.cpp generate.cpp ggs_archive_parser.cpp statistic_value_generator.cpp subboard.cpp value.cpp iddfs.cpp)
target_link_libraries(issen pthread)
