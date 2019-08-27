# Additional targets to perform clang-format/clang-tidy
# Get all project files
file(GLOB_RECURSE
     ALL_CXX_SOURCE_FILES
     include/*.[chi]pp include/*.[chi]xx include/*.cc include/*.hh include/*.ii include/*.[CHI]
     src/*.[chi]pp src/*.[chi]xx src/*.cc src/*.hh src/*.ii src/*.[CHI]
     )

find_package(GUROBI)

# TODO: find a prettier way to do this
# probably just make a function
if (NOT GUROBI_FOUND)
  get_filename_component(gurobi_cc src/gurobi/lpinterface_gurobi.cc ABSOLUTE)
  get_filename_component(gurobi_handle_cc src/gurobi/lphandle_gurobi.cc ABSOLUTE)
  get_filename_component(gurobi_hpp include/lpinterface/gurobi/lpinterface_gurobi.hpp ABSOLUTE)
  get_filename_component(gurobi_handle_hpp include/lpinterface/gurobi/lphandle_gurobi.hpp ABSOLUTE)
  get_filename_component(gurobi_util_hpp include/lpinterface/gurobi/lputil_gurobi.hpp ABSOLUTE)
  list(REMOVE_ITEM ALL_CXX_SOURCE_FILES "${gurobi_cc}")
  list(REMOVE_ITEM ALL_CXX_SOURCE_FILES "${gurobi_handle_cc}")
  list(REMOVE_ITEM ALL_CXX_SOURCE_FILES "${gurobi_hpp}")
  list(REMOVE_ITEM ALL_CXX_SOURCE_FILES "${gurobi_handle_hpp}")
  list(REMOVE_ITEM ALL_CXX_SOURCE_FILES "${gurobi_util_hpp}")
endif()

# Adding clang-format target if executable is found
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
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
    -checks='cppcoreguidelines-*,clang-diagnostic-weak-vtables'
    -warnings-as-errors=*
    --
    -I../include/
    -std=c++14
    )
endif()
