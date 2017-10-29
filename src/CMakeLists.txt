cmake_minimum_required(VERSION 2.8)
add_executable(issen bit_manipulations.cpp hand.cpp state.cpp utils.cpp main.cpp generate.cpp ggs_archive_parser.cpp subboard.cpp value.cpp gamesolver.cpp table.cpp move_generator.cpp movable_generator.cpp book.cpp)
target_link_libraries(issen pthread boost_system boost_timer)
