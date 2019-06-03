project (jpegturbo)

cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${CMAKE_JS_INC})

set(HEADER_FILES
        "src/exports.h"
    )
set(SOURCE_FILES
        "src/buffersize.cc"
        "src/compress.cc"
        "src/decompress.cc"
        "src/exports.cc"
    )

add_subdirectory("deps/libjpeg-turbo")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_include_directories(${PROJECT_NAME} PRIVATE "deps/libjpeg-turbo")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} turbojpeg-static)
