cmake_minimum_required(VERSION 3.9.2)
set(TARGET lpinterface)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual) 
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self)
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast)
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion)
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wsign-promo -Wstrict-overflow=5 -Wswitch-default)
set(CMAKE_CXX_FLAGS_LIB ${CMAKE_CXX_FLAGS_LIB} -Wno-unused -Werror -Wreturn-type -Wunused-variable -Wno-sign-compare)

set(lpinterface_files lp_impl.cc)

find_package(GUROBI)

if (GUROBI_FOUND)
    set(lpinterface_files ${lpinterface_files} ${CMAKE_CURRENT_SOURCE_DIR}/gurobi/lpinterface_gurobi.cc)
endif(GUROBI_FOUND)

add_library(${TARGET} STATIC ${lpinterface_files})

if (GUROBI_FOUND)
    target_link_libraries(${TARGET} gurobi81)
endif(GUROBI_FOUND)

target_compile_options(${TARGET} PRIVATE ${CMAKE_CXX_FLAGS_LIB})
