cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(root_kincalc)

#Specify the install prefix as the exec directory unless specified
file(MAKE_DIRECTORY exec)
get_filename_component(INSTALL_DIR ${CMAKE_SOURCE_DIR}/exec REALPATH)
set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH "Install Prefix" FORCE)
message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}")

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

message(STATUS "Processing through the source and include directories")
include_directories(${CMAKE_SOURCE_DIR}/include)
add_subdirectory(${CMAKE_SOURCE_DIR}/source)

install(FILES ${CMAKE_SOURCE_DIR}/share/mass_db.dat DESTINATION ${CMAKE_INSTALL_PREFIX})


