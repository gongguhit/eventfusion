# Version Information ---------------------------------------------------------
cmake_minimum_required(VERSION 2.8)
project(dvs)
set(CMAKE_CXX_STANDARD 11)

FIND_PACKAGE(dv-processing REQUIRED)

include_directories(
	"/usr/include"
	"/usr/local/include"
	)

add_executable(dvs dvstest.cpp)
# link your targets against the library
TARGET_LINK_LIBRARIES(dvs
	dv::processing
	)

