cmake_minimum_required(VERSION 2.8)
project(RoboticHandPneumatic)

# Find packages
find_package(OpenCV REQUIRED)

# Project source code
set(SOURCES
    src/main.cpp
)

# Include directories for libraries
set(INCLUDE_DIRS
    ${OpenCV_INCLUDE_DIRS}
)

# Libraries for linking
set(LIBRARIES
    ${OpenCV_LIBRARIES}
)

include_directories(include/)
include_directories(${INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS, "${CMAKE_CXX_FLAGS} -std=c++0x"})

# Compiling
add_executable(${PROJECT_NAME} ${SOURCES})

# Linking
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

