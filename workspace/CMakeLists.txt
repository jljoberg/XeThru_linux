cmake_minimum_required(VERSION 2.8)
project( XeThru_test )

include_directories(${PYTHONHOME}/include)
include_directories(${PYTHONHOME}/Lib/site-packages/numpy/core/include)
link_directories(${PYTHONHOME}/libs)

#add_definitions(-DMATPLOTLIBCPP_PYTHON_HEADER=Python.h)


# message(STATUS "*** dump start cmake variables ***")
# get_cmake_property(_variableNames VARIABLES)
# foreach(_variableName ${_variableNames})
#         message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()
# message(STATUS "*** dump end ***")sl

IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm") 
    add_definitions (-mfloat-abi=softfp -mfpu=neon) 
    set(XeThru_LIBDIR  ${PROJECT_SOURCE_DIR}/lib/ARM )
    message("ARM system detectec")
ELSE(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    set(XeThru_LIBDIR  ${PROJECT_SOURCE_DIR}/lib/x86 )
    message("Assuming x86")
ENDIF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")

message("Python HOME: " ${PYTHONHOME})

set(XeThru_INCLUDEDIR  ./include )
set(XeThru_LIBS  ModuleConnector)
set(XeTHRU_SRC ./src)

set(PLOT_INCLUDE ./plot_include /usr/include/python2.7 )
set(plot_LIBDIR)
set(PLOT_LIBS python2.7) 



include_directories( ${XeThru_INCLUDEDIR})
include_directories( ${PLOT_INCLUDE})
link_directories( ${XeThru_LIBDIR} )
add_executable(XeThru_test ${XeTHRU_SRC}/X4M03_test.cpp)
target_link_libraries(XeThru_test ${XeThru_LIBS} ${PLOT_LIBS})

message("CMAKE root: " ${PROJECT_SOURCE_DIR})
