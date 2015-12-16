project(cutsim)

cmake_minimum_required(VERSION 2.4)

if (CMAKE_BUILD_TOOL MATCHES "make")
    add_definitions(-Wall  -Wno-deprecated -Werror )
endif (CMAKE_BUILD_TOOL MATCHES "make")
# -pedantic-errors

FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtXml QtOpenGL REQUIRED)
INCLUDE(${QT_USE_FILE})
MESSAGE(STATUS "QT_USE_FILE = " ${QT_USE_FILE} )

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
    MESSAGE(STATUS "found OPENGL, lib = " ${OPENGL_LIBRARIES} )
endif(OPENGL_FOUND)

# find BOOST and boost-python
find_package( Boost )
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    MESSAGE(STATUS "found Boost: " ${Boost_LIB_VERSION})
    MESSAGE(STATUS "boost-incude dirs are: " ${Boost_INCLUDE_DIRS})
endif()

set( MOC_HEADERS 
    cutsim_app.hpp
    cutsim_window.hpp 
    text_area.hpp
    lex_analyzer.hpp
)
qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})

set( CUTSIM_SRC 
     ${${PROJECT_NAME}_SOURCE_DIR}/main.cpp 
     ${${PROJECT_NAME}_SOURCE_DIR}/text_area.cpp 
     ${${PROJECT_NAME}_SOURCE_DIR}/cutsim_window.cpp
     ${${PROJECT_NAME}_SOURCE_DIR}/lex_analyzer.cpp 
     ${MOC_OUTFILES}
)

################ create version_string.hpp, http://stackoverflow.com/questions/3780667
# include the output directory, where the version_string.hpp file is generated
include_directories(${CMAKE_CURRENT_BINARY_DIR})
# a custom target that is always built
add_custom_target(
    version_string 
    ALL
    DEPENDS ver_string 
)
# creates version_string.hpp using cmake script
add_custom_command(
    OUTPUT ver_string ${CMAKE_CURRENT_BINARY_DIR}/version_string.hpp
    COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
                                            -P ${CMAKE_CURRENT_SOURCE_DIR}/version_string.cmake
)
# version_string.hpp is a generated file
set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/version_string.hpp
    PROPERTIES GENERATED TRUE
    HEADER_FILE_ONLY TRUE
)
################ end create version_string.hpp

add_executable( 
    ${PROJECT_NAME} 
    ${CUTSIM_SRC}
)
add_dependencies( 
    ${PROJECT_NAME} 
    version_string 
)
target_link_libraries( 
    ${PROJECT_NAME} 
    libcutsim 
    g2m
    ${QT_LIBRARIES} 
    ${Boost_LIBRARIES} 
    ${OPENGL_LIBRARIES}
)

install( 
    TARGETS ${PROJECT_NAME} 
    DESTINATION bin 
)
