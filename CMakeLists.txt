cmake_minimum_required (VERSION 2.6)
project (font-effects-lib)

set(ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(HEADERS 
	${ROOT}/include/fe/fe_bundle.h
	${ROOT}/include/fe/fe_effect.h
	${ROOT}/include/fe/fe_export.h
	${ROOT}/include/fe/fe_gradient.h
	${ROOT}/include/fe/fe_image.h
	${ROOT}/include/fe/fe_node.h
	)

add_library(font-effect 	
	${ROOT}/src/fe_image.cpp
	${ROOT}/src/fe_effect.cpp
	${ROOT}/src/fe_node.cpp
	${ROOT}/src/fe_node_custom.cpp
	${ROOT}/src/fe_bundle.cpp
	${ROOT}/src/fe_gradient.cpp
	${ROOT}/src/ImageData.cpp
	${ROOT}/src/ImageDataOperations.cpp
	${HEADERS})

include_directories(${ROOT}/src)
include_directories(${ROOT}/include)

add_executable(font-effect-test test/main.cpp test/tga.h test/tga.cpp)
target_link_libraries(font-effect-test font-effect)