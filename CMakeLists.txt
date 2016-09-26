cmake_minimum_required(VERSION 2.6)
project(hyread)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#QWT start
#If you don't want the hassle of including Qwt, comment out the next few lines until QWT stop
ADD_DEFINITIONS(-DWITH_QWT)
set(LIBS qwt-qt5)
INCLUDE_DIRECTORIES(/usr/include/qwt)
#QWT stop

set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets)

#compile and link
add_executable(hyview src/main.cpp src/readimage.cpp src/imageViewer.cpp)
TARGET_LINK_LIBRARIES(hyview Qt5::Widgets ${LIBS})

#install
install (TARGETS hyview DESTINATION bin)


