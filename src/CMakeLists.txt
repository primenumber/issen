cmake_minimum_required(VERSION 2.8)
add_executable(issen bit_manipulations.cpp gcc_intrinsics.cpp hand.cpp line.cpp state.cpp utils.cpp main.cpp generate.cpp ggs_archive_parser.cpp play.cpp statistic_value_generator.cpp subboard.cpp tree.cpp tree_manager.cpp treesearch.cpp value.cpp iddfs.cpp)
target_link_libraries(issen pthread)
