cmake_minimum_required(VERSION 2.9)
project(lpinterface)
set(TARGET lpinterface)


set(COVERAGE OFF CACHE BOOL "Coverage")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
  set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wno-unused-command-line-argument)
endif()

# Need this because Travis doesn't work well with
# CMAKE_CXX_STANDARD
if (CI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
set(CMAKE_CXX_EXTENSIONS OFF)

# Library specific compiler flags
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual) 
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self)
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast)
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion)
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wsign-promo -Wstrict-overflow=5 -Wswitch-default)
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wno-unused -Wno-unused-parameter -Werror -Wreturn-type -Wunused-variable -Wno-sign-compare)

# Cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# include dirs
include_directories(lib)
include_directories(include)
include_directories(include/lpinterface/detail)

# clang-format and clang-tidy
include(cmake/clang-cxx-dev-tools.cmake)

# source files
#list(APPEND lpinterface_files src/lp_impl.cc)

# Optional dependencies
find_package(GUROBI)

if (GUROBI_FOUND)
  message(STATUS "Found Gurobi: TRUE")
  include_directories(${GUROBI_INCLUDE_DIRS})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/gurobi)
  list(APPEND LIBS ${GUROBI_LIBRARY})
  list(APPEND lpinterface_files 
        src/gurobi/lpinterface_gurobi.cc
        src/gurobi/lphandle_gurobi.cc)
endif(GUROBI_FOUND)

find_package(SOPLEX)

if (SOPLEX_FOUND)
  message(STATUS "Found SoPlex: TRUE")
  include_directories(${SOPLEX_INCLUDE_DIRS})
  list(APPEND LIBS ${SOPLEX_LIBRARIES})
  list(APPEND lpinterface_files 
        src/soplex/lpinterface_soplex.cc
        src/soplex/lphandle_soplex.cc)
endif(SOPLEX_FOUND)

# Gtest configuration
if (LPINT_ENABLE_TESTING)
  include(cmake/gtest.cmake)
  include(cmake/gmock.cmake)
  include_directories(SYSTEM lib/rapidcheck/extras/gtest/include)
  include_directories(SYSTEM lib/rapidcheck/include)
  add_subdirectory(lib/rapidcheck)
  add_subdirectory(tests)
endif()

add_library(${TARGET} ${lpinterface_files})

# link optional dependencies
target_link_libraries(${TARGET} ${LIBS})

# set library compiler options
target_compile_options(${TARGET} PRIVATE ${CMAKE_CXX_FLAGS_LIB})

# expose library include directories
target_include_directories(lpinterface PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include>  # <prefix>/include
)

# compile examples if desired
if (LPINT_ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()