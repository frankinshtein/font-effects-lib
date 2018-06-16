cmake_minimum_required (VERSION 2.6)

project(font-effects)

OPTION(FE_EXTERNAL_BUILD "FE_EXTERNAL_BUILD" OFF)
option(FE_TEST_BUILD "FE_TEST_BUILD" ON)

IF( NOT FE_EXTERNAL_BUILD )
    SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_GENERATOR} )    
ENDIF()

SET( INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include )
SET( SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src )

set(HEADERS 
	${INCLUDE_DIR}/fe/fe_bundle.h
	${INCLUDE_DIR}/fe/fe_effect.h
	${INCLUDE_DIR}/fe/fe_export.h
	${INCLUDE_DIR}/fe/fe_gradient.h
	${INCLUDE_DIR}/fe/fe_image.h
	${INCLUDE_DIR}/fe/fe_node.h
	)
    
set(SOURCES
	${SOURCE_DIR}/fe_image.cpp
	${SOURCE_DIR}/fe_effect.cpp
	${SOURCE_DIR}/fe_node.cpp
	${SOURCE_DIR}/fe_node_custom.cpp
	${SOURCE_DIR}/fe_node_effects.cpp
	${SOURCE_DIR}/fe_bundle.cpp
	${SOURCE_DIR}/fe_gradient.cpp
	${SOURCE_DIR}/ImageData.cpp
	${SOURCE_DIR}/ImageDataOperations.cpp
    )

include_directories(${INCLUDE_DIR})
    
add_library(${PROJECT_NAME}
    ${HEADERS}
	${SOURCES}	
    )
    
if( FE_TEST_BUILD )
    SET( TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test )
    
    set(TEST_SOURCES
        ${TEST_DIR}/main.cpp
        ${TEST_DIR}/tga.h
        ${TEST_DIR}/tga.cpp
        )
        
    include_directories(${INCLUDE_DIR})
    include_directories(${SOURCE_DIR})

    add_executable(test ${TEST_SOURCES})
    target_link_libraries(test font-effects)
endif()