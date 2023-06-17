# Version Information ---------------------------------------------------------
cmake_minimum_required(VERSION 2.8)
project(dvs)
set(CMAKE_CXX_STANDARD 11)

FIND_PACKAGE(dv-processing REQUIRED)

include_directories(
	"/usr/include"
	"/usr/local/include"
	)

# Set the source directory
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add all the .cpp files in the current directory
file(GLOB SRC_FILES ${SRC_DIR}/*.cpp)

# Create a bin directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Loop over each source file and create an executable for each one
foreach(SRC_FILE ${SRC_FILES})
    # Get the name of the source file without the extension
    get_filename_component(EXEC_NAME ${SRC_FILE} NAME_WE)
    # Add an executable with the same name as the source file
    add_executable(${EXEC_NAME} ${SRC_FILE})
    # Link the executable with the required libraries
    target_link_libraries(${EXEC_NAME} dv::processing)
    # Specify the output directory for the executable
    set_target_properties(${EXEC_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

endforeach()

#add_executable(dvs dvstest.cpp)
# link your targets against the library
#TARGET_LINK_LIBRARIES(dvs
#	dv::processing
#	)

