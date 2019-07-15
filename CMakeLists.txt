cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

include_directories(src)
include_directories(lib)
include_directories(include)

include(cmake/clang-cxx-dev-tools.cmake)

add_subdirectory(src)

if (TESTING)
  include(cmake/gtest.cmake)
  include(cmake/gmock.cmake)
  include_directories(SYSTEM lib/rapidcheck/extras/gtest/include)
  include_directories(SYSTEM lib/rapidcheck/include)
  add_subdirectory(lib/rapidcheck)
  add_subdirectory(tests)
endif()
