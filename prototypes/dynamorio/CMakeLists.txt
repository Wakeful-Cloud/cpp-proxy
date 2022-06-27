# Configure CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(proxy)

# Find DynamoRIO (See https://dynamorio.org/page_build_client.html)
find_package(DynamoRIO)
if (NOT DynamoRIO_FOUND)
  message(FATAL_ERROR "DynamoRIO package required to build")
endif(NOT DynamoRIO_FOUND)

# Add the proxy
add_library(proxy SHARED src/proxy.c)

# Add and configure DynamoRIO
set(DynamoRIO_USE_LIBC OFF)
configure_DynamoRIO_client(proxy)
use_DynamoRIO_extension(proxy drmgr)
use_DynamoRIO_extension(proxy drsyms)
use_DynamoRIO_extension(proxy drwrap)