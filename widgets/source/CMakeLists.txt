
set(WidgetSources QZMainframe.cxx QZPlotwidget.cxx)
add_library(WidgetSourcesObj OBJECT ${WidgetSources})
add_library(WidgetSourcesLib SHARED $<TARGET_OBJECTS:WidgetSourcesObj>)
target_link_libraries(WidgetSourcesLib KinLib )

install(TARGETS WidgetSourcesLib DESTINATION ${CMAKE_INSTALL_PREFIX}/libs)
