cmake_minimum_required(VERSION 2.8)
project(RoboticHandPneumatic)

# Find packages
find_package(OpenCV REQUIRED)
find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)

# Set C++ flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

# Project source code
set(SOURCES
    src/main.cpp
    src/zexception.cpp
    src/mainwindow.cpp
    src/whandgesture.cpp
)

# Headers for MOC
set(HEADERS
    include/zexception.h
    include/mainwindow.h
    include/whandgesture.h
)

# Include directories for libraries
set(INCLUDE_DIRS
    3rdpart/
    ${OpenCV_INCLUDE_DIRS}
    ${QT_INCLUDES}
)

# Libraries for linking
set(LIBRARIES
    ${OpenCV_LIBRARIES}
    ${QT_QTCORE_LIBRARIES}
    ${QT_QTGUI_LIBRARIES}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdpart/libqextserialport.so # Must be better!
)

include_directories(include/)
include_directories(${INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include/)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/3rdpart/)

# MOC Compiling
foreach(MOCHeader ${HEADERS})
    qt4_wrap_cpp(MOCSrc ${MOCHeader})
endforeach(MOCHeader)

# Compiling
add_executable(${PROJECT_NAME} ${SOURCES} ${MOCSrc} ${3RDPART_SOURCES} ${3RDPART_MOC_HEADERS})

# Linking
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
