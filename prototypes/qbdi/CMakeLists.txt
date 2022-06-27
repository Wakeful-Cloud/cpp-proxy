# Configure CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(proxy)

# Options
option(VERBOSE "Whether or not verbose logging is enabled" OFF)

# Find QBDI (See https://qbdi.readthedocs.io/en/stable/installation_and_integration.html?highlight=cmake#single-architecture)
find_package(QBDI REQUIRED)
find_package(QBDIPreload REQUIRED)

# Add the library
include_directories(../../src)
file(GLOB LIB "../../src/*")

if(VERBOSE)
  add_definitions(-D VERBOSE)
endif(VERBOSE)

# Add the executable
add_library(proxy SHARED src/proxy.cpp ${LIB})

# Add elfutils
target_link_libraries(proxy elf)

# Add QBDI
target_link_libraries(proxy QBDIPreload::QBDIPreload QBDI::QBDI_static)