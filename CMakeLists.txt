cmake_minimum_required(VERSION 2.6)
project(hyread)
FIND_PACKAGE(Qt4 REQUIRED)
set(CMAKE_BUILD_TYPE Release)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#QWT start
#If you don't want the hassle of including Qwt, comment out the next few lines until QWT stop
ADD_DEFINITIONS(-DWITH_QWT)
set(LIBS qwt-qt4)
INCLUDE_DIRECTORIES(/usr/include/qwt-qt4)
#QWT stop


#QT processing
set(QT_HEADERS include/imageViewer.h)
QT4_WRAP_CPP(QT_HEADERS_MOC ${QT_HEADERS})
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

#compile and link
add_executable(hyview src/main.cpp src/readimage.cpp src/imageViewer.cpp ${QT_HEADERS_MOC})
TARGET_LINK_LIBRARIES(hyview ${QT_LIBRARIES} ${LIBS})

#install
install (TARGETS hyview DESTINATION bin)


