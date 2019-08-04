add_executable(lp_simple lp_simple.cc)
add_executable(nqueens nqueens.cc)

target_link_libraries(lp_simple lpinterface)
target_link_libraries(nqueens lpinterface)
