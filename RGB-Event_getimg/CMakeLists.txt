cmake_minimum_required(VERSION 3.25)
project(RGB_Event_getimg)

set(CMAKE_CXX_STANDARD 11)

SET (PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
find_package(OpenCV REQUIRED)
find_package(OpenGL)
#find_package(PCL)
find_package(realsense2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)


include_directories(
        ${PROJECT_INCLUDE_DIR}
        "/usr/local/include/")
link_directories(
        /home/emrys/CLionProjects/RGB-Event_getimg/lib)


add_executable(RGB_Event_getimg main.cpp)

target_link_libraries(RGB_Event_getimg
        realsense2::realsense2
        OpenGL::OpenGL
        glfw
        CeleX
#        ${PCL_LIBRARIES}
        ${OpenCV_LIBS}
        ${OPENGL_LIBRARIES}
        ${GLUT_LIBRARY}
        )
