
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC  -std=gnu++11")

#---Create a shared library with geneated dictionary
set(KinSources kinematic.cxx CalcMonitor.cxx)
add_library(RootKinObj OBJECT ${KinSources})
add_library(RootKinLib SHARED $<TARGET_OBJECTS:RootKinObj>)
target_link_libraries(RootKinLib QCustomPlotLibs)

install(TARGETS RootKinLib DESTINATION ${CMAKE_INSTALL_PREFIX}/libs)

#if (${ROOT_VERSION} VERSION_GREATER "6.0")
#	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/RootKinDict_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX})
#endif()