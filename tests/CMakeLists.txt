cmake_minimum_required(VERSION 2.9)
set(UNIT_TESTS unit_tests)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
  set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wno-unused-command-line-argument)
endif()

if (CI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
else()
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual) 
set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self)
set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast)
set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion)
set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wsign-promo -Wstrict-overflow=5 -Wswitch-default)
set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wno-unused -Wno-unused-parameter -Werror -Wsign-compare)

find_package(Threads REQUIRED)

set(test_files
  test.cc
  test_solvers.cc
  test_data_objects.cc)

list(APPEND LIBS lpinterface)

find_package(GUROBI)
if (GUROBI_FOUND)
  list(APPEND test_files test_gurobi.cc)
  include_directories(${GUROBI_INCLUDE_DIRS})
  list(APPEND LIBS ${GUROBI_LIBRARY})

  add_definitions(-DLPINT_GUROBI_SUPPORTED)
  list(APPEND SUPPORTED_SOLVERS GurobiSolver)
endif (GUROBI_FOUND)

find_package(SOPLEX)
if (SOPLEX_FOUND)
  list(APPEND test_files test_soplex.cc)
  include_directories(${SOPLEX_INCLUDE_DIRS})
  list(APPEND LIBS ${SOPLEX_LIBRARIES})

  list(APPEND SUPPORTED_SOLVERS SoplexSolver)
  add_definitions(-DLPINT_SOPLEX_SUPPORTED)
endif (SOPLEX_FOUND)

find_package(Cplex)
if (CPLEX_FOUND)
  #list(APPEND test_files test_soplex.cc)
  include_directories(${CPLEX_INCLUDE_DIRS})
  list(APPEND LIBS ${CPLEX_LIBRARIES})

  list(APPEND SUPPORTED_SOLVERS CplexSolver)
  add_definitions(-DLPINT_CPLEX_SUPPORTED)
endif (CPLEX_FOUND)

string(REPLACE ";" "," SUPPORTED_SOLVERS "${SUPPORTED_SOLVERS}")

add_definitions(-DLPINT_SUPPORTED_SOLVERS=${SUPPORTED_SOLVERS})

add_executable(${UNIT_TESTS} ${test_files})

list(APPEND LIBS gtest gmock Threads::Threads rapidcheck)

target_link_libraries(${UNIT_TESTS} PUBLIC ${LIBS})

target_compile_options(${UNIT_TESTS} PRIVATE ${CMAKE_CXX_FLAGS_TESTS})

add_custom_command(TARGET ${UNIT_TESTS} POST_BUILD 
    COMMAND ${CMAKE_BINARY_DIR}/tests/${UNIT_TESTS} --gtest_shuffle
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running tests")

if (COVERAGE) 
  target_compile_options(${UNIT_TESTS} PRIVATE --coverage -g -O0
                                               -fprofile-arcs -ftest-coverage)
  target_link_libraries(${UNIT_TESTS} PRIVATE --coverage -g)
endif()
