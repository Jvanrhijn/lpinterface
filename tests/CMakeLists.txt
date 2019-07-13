cmake_minimum_required(VERSION 3.9)
set(UNIT_TESTS unit_tests)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual) 
set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self)
set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast)
set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion)
set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wsign-promo -Wstrict-overflow=5 -Wswitch-default)
set(CMAKE_CXX_FLAGS_TESTS ${CMAKE_CXX_FLAGS_TESTS} -Wno-unused)

find_package(Threads REQUIRED)

set(test_files
  test.cc)

add_executable(${UNIT_TESTS} ${test_files})
target_link_libraries(${UNIT_TESTS} PUBLIC lpinterface gtest gmock Threads::Threads rapidcheck)
target_compile_options(${UNIT_TESTS} PRIVATE ${CMAKE_CXX_FLAGS_TESTS})
