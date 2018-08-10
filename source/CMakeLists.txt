#---Create a shared library with geneated dictionary
set(KinSources kinematic.cxx CalcMonitor.cxx)
add_library(KinObj OBJECT ${KinSources})
add_library(KinLib SHARED $<TARGET_OBJECTS:KinObj>)
target_link_libraries(KinLib QCustomPlotLibs)

install(TARGETS KinLib DESTINATION ${CMAKE_INSTALL_PREFIX}/libs)
