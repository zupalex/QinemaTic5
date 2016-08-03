
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

ROOT_GENERATE_DICTIONARY(RootKinDict kinematic.h LINKDEF kinematicLinkDef.h)

#---Create a shared library with geneated dictionary
set(KinSources kinematic.cxx RootKinDict.cxx)
add_library(RootKinObj OBJECT ${KinSources})
add_library(RootKinLib SHARED $<TARGET_OBJECTS:RootKinObj>)
target_link_libraries(RootKinLib ${ROOT_LIBRARIES})

#---Create  a main program using the library
add_executable(RootKinCalc ExecKinCalc.cxx $<TARGET_OBJECTS:RootKinObj>)
target_link_libraries(RootKinCalc RootKinLib)

install(TARGETS RootKinCalc DESTINATION ${CMAKE_INSTALL_PREFIX})

install(TARGETS RootKinLib DESTINATION ${CMAKE_INSTALL_PREFIX})

if (${ROOT_VERSION} VERSION_GREATER "6.0")
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/RootKinDict_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()