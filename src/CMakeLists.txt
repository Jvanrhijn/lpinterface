cmake_minimum_required(VERSION 3.9.2)

set(lpinterface_files lpinterface_gurobi.cc)

add_library(lpinterface STATIC ${lpinterface_files})
