
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC  -std=gnu++11")

#---Create a shared library with geneated dictionary
set(KinSources kinematic.cxx CalcMonitor.cxx)
add_library(KinObj OBJECT ${KinSources})
add_library(KinLib SHARED $<TARGET_OBJECTS:KinObj>)
target_link_libraries(KinLib QCustomPlotLibs)

install(TARGETS KinLib DESTINATION ${CMAKE_INSTALL_PREFIX}/libs)

#if (${ROOT_VERSION} VERSION_GREATER "6.0")
#	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/RootKinDict_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX})
#endif()
