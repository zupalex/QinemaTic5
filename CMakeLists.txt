cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(root_kincalc)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

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

#Find the ROOT package and configure the flags and paths
message(STATUS "Finding ROOT packages")
find_package (ROOT REQUIRED)
message(STATUS "cmake cxx flags set to ${CMAKE_CXX_FLAGS} ${ROOT_CXX_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CXX_FLAGS}")
message(STATUS "ROOT Include Dir: ${ROOT_INCLUDE_DIR}")
include_directories(${ROOT_INCLUDE_DIR})
link_directories(${ROOT_LIBRARY_DIR})

message(STATUS "If the build process failed to find the Qt5 packages, install the Qt5 packages (e.g. \"qtbase5-dev\" for Ubuntu and Debian, \"qt5\" for Mac OSX, \"qt5-qtbase-devel\" on Fedora, ...)")

message(STATUS "Finding Qt5 packages")
find_package(Qt5Widgets REQUIRED)

include_directories(${Qt5Widgets_INCLUDE_DIRS})

message(STATUS "Processing through the source and include directories")
include_directories(${CMAKE_SOURCE_DIR}/include)
add_subdirectory(${CMAKE_SOURCE_DIR}/source)

include_directories(widgets/include)
add_subdirectory(widgets)

#---Create the Qt based main program using the library
add_executable(QtRootKinCalc ${CMAKE_SOURCE_DIR}/source/ExecKinCalcQt.cxx)
target_link_libraries(QtRootKinCalc WidgetSourcesLib)

install(TARGETS QtRootKinCalc DESTINATION ${CMAKE_INSTALL_PREFIX})

install(FILES ${CMAKE_SOURCE_DIR}/share/mass_db.dat DESTINATION ${CMAKE_INSTALL_PREFIX})


