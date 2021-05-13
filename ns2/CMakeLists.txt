cmake_minimum_required(VERSION 3.6)
project(ns)
set(TARNAME ns)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# program name, version etc
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(PACKAGE_VERSION "2.35")
set(PACKAGE_NAME "ns")
set(PACKAGE_TARNAME "${TARNAME}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "http://sourceforge.net/projects/nsnam")

find_package(PkgConfig REQUIRED)

##########################################################################
if(NOT DEFINED BIN_INSTALL_DIR)
    set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
endif(NOT DEFINED BIN_INSTALL_DIR)
if(NOT DEFINED DATA_INSTALL_DIR)
    set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share")
endif(NOT DEFINED DATA_INSTALL_DIR)
if(NOT DEFINED MAN_INSTALL_DIR)
    set(MAN_INSTALL_DIR "${DATA_INSTALL_DIR}/man")
endif(NOT DEFINED MAN_INSTALL_DIR)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_options(-ggdb -O0 -pg)
endif()

add_definitions(-DHAVE_CONFIG_H)
add_definitions(-DCPP_NAMESPACE=std)
add_definitions(-DSTL_NAMESPACE=std)
add_definitions(-DUSE_INTERP_ERRORLINE)
add_definitions(-DUSE_INTERP_RESULT)
INCLUDE(CheckModules)
#######################################################################
## common/ptypes2tcl
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common/
)

set(ptypes2tcl_SRC
    common/ptypes2tcl.cc
)

add_executable(ptypes2tcl ${ptypes2tcl_SRC})
target_link_libraries(ptypes2tcl)

#######################################################################
### source code files
include(SourceFiles)
################################################################
### ns 
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${INCLUDE_SUB_DIR}
)
include_directories(
    SYSTEM
    ${TCL_INCLUDE_PATH}
    ${TK_INCLUDE_PATH}
    ${OTCL_INCLUDE_DIRS}
    ${TCLCL_INCLUDE_DIRS}
    ${SCHAD_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}
)

add_subdirectory(queue/learning)

set(ns_SRC
    common/tclAppInit.cc
    common/main-monolithic.cc
    ${OBJ}
)

add_executable(ns ${ns_SRC} $<TARGET_OBJECTS:learning>)
target_link_libraries(ns -lnsl -ldl -lm ${X11_LIBRARIES} ${X11_Xext_LIB}
        ${TCL_LIBRARY} ${TCL_STUB_LIBRARY} ${TK_LIBRARY} ${TK_STUB_LIBRARY}
        ${OTCL_LIBRARIES} ${TCLCL_LIBRARIES} ${Boost_LIBRARIES} ${SCHAD_LIBRARIES})
set_property(TARGET ns APPEND PROPERTY LINK_FLAGS_DEBUG -pg)
install(TARGETS ns RUNTIME DESTINATION ${BIN_INSTALL_DIR})

install(FILES ns.1 DESTINATION ${MAN_INSTALL_DIR}/man1)


#######################################################################################
### nse
include_directories(
    SYSTEM
    ${TCL_INCLUDE_PATH}
    ${TK_INCLUDE_PATH}
    ${OTCL_INCLUDE_DIRS}
    ${TCLCL_INCLUDE_DIRS}
	${PCAP_INCLUDE_DIRS}
)
include_directories(
    ${INCLUDE_SUB_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
set(nse_SRC
    common/tclAppInit.cc
    common/main-monolithic.cc
    ${OBJ}
    ${OBJ_EMULATE_CC}
    ${OBJ_EMULATE_C}
)
add_executable(nse ${nse_SRC})
target_link_libraries(nse -lnsl -ldl -lm ${X11_LIBRARIES} ${X11_Xext_LIB} ${PCAP_LIBRARIES} ${TCL_LIBRARY} ${TCL_STUB_LIBRARY} ${TK_LIBRARY} ${TK_STUB_LIBRARY} ${OTCL_LIBRARIES} ${TCLCL_LIBRARIES})
install(TARGETS nse RUNTIME DESTINATION ${BIN_INSTALL_DIR})

#######################################################################################
### nstk
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${TCL_INCLUDE_PATH}
    ${TK_INCLUDE_PATH}
    ${OTCL_INCLUDE_DIRS}
    ${TCLCL_INCLUDE_DIRS}
    ${INCLUDE_SUB_DIR}
)
set(nstk_SRC
    common/tkAppInit.cc
    ${OBJ}
)
add_executable(nstk ${nstk_SRC})
target_link_libraries(nstk ${TCL_LIBRARY} ${TCL_STUB_LIBRARY} ${TK_LIBRARY} ${TK_STUB_LIBRARY} ${OTCL_LIBRARIES} ${TCLCL_LIBRARIES})
install(TARGETS nstk RUNTIME DESTINATION ${BIN_INSTALL_DIR})

#########################################################################################
####  indep-utils
add_subdirectory(indep-utils/cmu-scen-gen/setdest)
add_subdirectory(indep-utils/webtrace-conv/dec)
add_subdirectory(indep-utils/webtrace-conv/epa)
add_subdirectory(indep-utils/webtrace-conv/nlanr)
add_subdirectory(indep-utils/webtrace-conv/ucb)
#=========================================================================================
### test
enable_testing()
add_test(NAME ns_TEST COMMAND ./validate)
