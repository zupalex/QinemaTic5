set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC  -std=gnu++11")

add_library(QCustomPlotLibs SHARED qcustomplot.cpp ../include/qcustomplot.h)
target_link_libraries(QCustomPlotLibs Qt5::Widgets Qt5::PrintSupport)

install(TARGETS QCustomPlotLibs DESTINATION ${CMAKE_INSTALL_PREFIX}/libs)