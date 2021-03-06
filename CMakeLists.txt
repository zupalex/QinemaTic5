cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(root_kincalc)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#Specify the install prefix as the exec directory unless specified
file(MAKE_DIRECTORY exec)
file(MAKE_DIRECTORY exec/libs)

get_filename_component(INSTALL_DIR ${CMAKE_SOURCE_DIR}/exec REALPATH)
set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH "Install Prefix" FORCE)
message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}")

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/libs")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

#Add custom modules to module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

message(STATUS "Finding Qt5 packages")
find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)

include_directories(${Qt5Widgets_INCLUDE_DIRS})

message(STATUS "This software is using QCustomPlot by Emanuel Eichhammer")
message(STATUS "QCustomPlot website: http://www.qcustomplot.com/index.php/")
include_directories(qcustomplot/include)
add_subdirectory(qcustomplot)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wno-unused-parameter -Wno-unused-but-set-variable -pedantic -fPIC -std=c++11 -g")


message(STATUS "If the build process failed to find the Qt5 packages, install the Qt5 packages (e.g. \"qtbase5-dev\" for Ubuntu and Debian, \"qt5\" for Mac OSX, \"qt5-qtbase-devel\" on Fedora, ...)")

message(STATUS "Processing through the source and include directories")
include_directories(${CMAKE_SOURCE_DIR}/include)
add_subdirectory(${CMAKE_SOURCE_DIR}/source)

include_directories(widgets/include)
add_subdirectory(widgets)

#---Create the Qt based main program using the library
add_executable(QinemaTic5 ${CMAKE_SOURCE_DIR}/source/ExecKinCalcQt.cxx ${CMAKE_SOURCE_DIR}/share/images-resources.qrc)
target_link_libraries(QinemaTic5 QCustomPlotLibs WidgetSourcesLib)

install(TARGETS QinemaTic5 DESTINATION ${CMAKE_INSTALL_PREFIX})

install(FILES ${CMAKE_SOURCE_DIR}/share/mass_db.dat DESTINATION ${CMAKE_INSTALL_PREFIX})


