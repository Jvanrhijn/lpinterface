# Additional targets to perform clang-format/clang-tidy
# Get all project files
file(GLOB_RECURSE
     ALL_CXX_SOURCE_FILES
     include/*.[chi]pp include/*.[chi]xx include/*.cc include/*.hh include/*.ii include/*.[CHI]
     src/*.[chi]pp src/*.[chi]xx src/*.cc src/*.hh src/*.ii src/*.[CHI]
     )

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
    -checks='cppcoreguidelines-*'
    -warnings-as-errors=*
    --
    -I../include/
    -std=c++14
    )
endif()
