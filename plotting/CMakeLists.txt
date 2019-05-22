cmake_minimum_required(VERSION 2.8)
project( plot_XeThru )

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

message("Python HOME: " ${PYTHONHOME})

set(PLOT_INCLUDE ./plot_include /usr/include/python2.7 )
set(plot_LIBDIR)
set(PLOT_LIBS python2.7) 


include_directories( ${PLOT_INCLUDE})
add_executable(plot_XeThru ./plot_XeThru.cpp)
target_link_libraries(plot_XeThru ${XeThru_LIBS} ${PLOT_LIBS})

message("CMAKE root: " ${PROJECT_SOURCE_DIR})
