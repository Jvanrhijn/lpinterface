# Additional targets to perform clang-format/clang-tidy
# Get all project files
file(GLOB_RECURSE
     ALL_CXX_SOURCE_FILES
     include/*.[chi]pp include/*.[chi]xx include/*.cc include/*.hh include/*.ii include/*.[CHI]
     src/*.[chi]pp src/*.[chi]xx src/*.cc src/*.hh src/*.ii src/*.[CHI]
     tests/test_*.cc tests/test.cc tests/generators.hpp tests/mock_*.hpp
     )

find_package(GUROBI)

if (NOT GUROBI_FOUND)
  get_filename_component(gurobi_cc src/gurobi/lpinterface_gurobi.cc ABSOLUTE)
  get_filename_component(gurobi_hpp include/lpinterface/gurobi/lpinterface_gurobi.hpp ABSOLUTE)
  list(REMOVE_ITEM ALL_CXX_SOURCE_FILES "${gurobi_cc}")
  list(REMOVE_ITEM ALL_CXX_SOURCE_FILES "${gurobi_hpp}")
endif()

# Adding clang-format target if executable is found
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
  message("Source files: ${ALL_CXX_SOURCE_FILES}")
  add_custom_target(
    clang-format
    COMMAND /usr/bin/clang-format
    -i
    -style=Google
    ${ALL_CXX_SOURCE_FILES}
    )
endif()

# Adding clang-tidy target if executable is found
find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
  add_custom_target(
    clang-tidy
    COMMAND clang-tidy
    ${ALL_CXX_SOURCE_FILES}
    -checks='cppcoreguidelines-*'
    -warnings-as-errors=*
    --
    -I../include/
    -std=c++14
    )
endif()
