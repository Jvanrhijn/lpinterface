cmake_minimum_required(VERSION 3.9)
project(lpinterface)
set(TARGET lpinterface)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COVERAGE OFF CACHE BOOL "Coverage")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual) 
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self)
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast)
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion)
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wsign-promo -Wstrict-overflow=5 -Wswitch-default)
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wno-unused -Werror -Wreturn-type -Wunused-variable -Wno-sign-compare)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

include_directories(lib)
include_directories(include)

include(cmake/clang-cxx-dev-tools.cmake)

find_package(GUROBI)

if (GUROBI_FOUND)
    message(STATUS "Found Gurobi: TRUE")
    include_directories(${GUROBI_INCLUDE_DIRS})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/gurobi)
endif(GUROBI_FOUND)

if (TESTING)
  include(cmake/gtest.cmake)
  include(cmake/gmock.cmake)
  include_directories(SYSTEM lib/rapidcheck/extras/gtest/include)
  include_directories(SYSTEM lib/rapidcheck/include)
  add_subdirectory(lib/rapidcheck)
  add_subdirectory(tests)
endif()

set(lpinterface_files 
        src/lp_impl.cc)

if (GUROBI_FOUND)
    set(lpinterface_files ${lpinterface_files} 
            src/gurobi/lpinterface_gurobi.cc)
endif(GUROBI_FOUND)

add_library(${TARGET} STATIC ${lpinterface_files})

if (GUROBI_FOUND)
    target_link_libraries(${TARGET} gurobi81)
endif(GUROBI_FOUND)

target_compile_options(${TARGET} PRIVATE ${CMAKE_CXX_FLAGS_LIB})

target_include_directories(lpinterface PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include>  # <prefix>/include
)

if (ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()