cmake_minimum_required(VERSION 2.8)
include_directories(.)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

add_subdirectory(cutsim)
add_subdirectory(g2m)
add_subdirectory(app)


# find doxygen, and add a "doc" target which builds html-documentation
FIND_PACKAGE(Doxygen)
IF (DOXYGEN_FOUND)

    if (EXISTS ${CMAKE_SOURCE_DIR}/Doxyfile )
        set(DOXY_CONFIG ${CMAKE_SOURCE_DIR}/Doxyfile )
        MESSAGE(STATUS " DOXY_CONFIG = " ${DOXY_CONFIG} )
    endif (EXISTS ${CMAKE_SOURCE_DIR}/Doxyfile)
    
    MESSAGE(STATUS " Run 'make doc' to generate documentation.")
    add_custom_target(
        doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG}
        COMMENT building HTML docs
    )
        
ENDIF(DOXYGEN_FOUND)
