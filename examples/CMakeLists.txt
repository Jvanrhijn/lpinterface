set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lp_simple lp_simple.cc)

target_link_libraries(lp_simple lpinterface)
