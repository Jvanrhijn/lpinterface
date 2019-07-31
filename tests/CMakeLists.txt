cmake_minimum_required(VERSION 3.9)
set(UNIT_TESTS unit_tests)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual) 
set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self)
set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast)
set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion)
set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wsign-promo -Wstrict-overflow=5 -Wswitch-default)
set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wno-unused -Werror -Wsign-compare)

find_package(Threads REQUIRED)

set(test_files
  test.cc
  test_sparse_matrix.cc)

find_package(GUROBI)
if (GUROBI_FOUND)
  list(APPEND test_files test_gurobi.cc)
endif (GUROBI_FOUND)

find_package(SOPLEX)
if (SOPLEX_FOUND)
  list(APPEND test_files test_soplex.cc)
endif (SOPLEX_FOUND)

add_executable(${UNIT_TESTS} ${test_files})

if (GUROBI_FOUND)
  target_link_libraries(${UNIT_TESTS} PUBLIC lpinterface gurobi81 gtest gmock Threads::Threads rapidcheck)
else()
  target_link_libraries(${UNIT_TESTS} PUBLIC lpinterface gtest gmock Threads::Threads rapidcheck)
endif(GUROBI_FOUND)

target_compile_options(${UNIT_TESTS} PRIVATE ${CMAKE_CXX_FLAGS_TESTS})

add_custom_command(TARGET ${UNIT_TESTS} POST_BUILD 
    COMMAND ${CMAKE_BINARY_DIR}/tests/${UNIT_TESTS}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running tests")

if (COVERAGE) 
  target_compile_options(${UNIT_TESTS} PRIVATE --coverage -g)
  target_link_libraries(${UNIT_TESTS} PRIVATE --coverage -g)
endif()
