cmake_minimum_required(VERSION 3.17)
project(ors)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavdevice
    libavfilter
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil
)

find_package(Freetype REQUIRED)

if(CMAKE_HOST_APPLE)
    add_compile_options(-Wno-deprecated-declarations)
endif(CMAKE_HOST_APPLE)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/glfw)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/glfw/include)
LIST(APPEND LIBS glfw ${GLFW_LIBRARIES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/glad/include)
set(GLAD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/glad/src/glad.c)
if(NOT WIN32)
    LIST(APPEND LIBS dl)
endif()

include_directories(3rd_party/glm)

add_subdirectory(3rd_party/librtc)

add_executable(
        ors
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/encoder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rtmp_publisher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/streamer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/scene.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rtc_publisher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dispatchqueue.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/h264fileparser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/stream.cpp
        ${GLAD_SRC}
)

find_package(Threads REQUIRED)
#target_include_directories(pic2video PRIVATE ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${AVDEVICE_INCLUDE_DIR})
target_link_libraries(ors ${LIBS} PkgConfig::LIBAV LibDataChannel::LibDataChannel Threads::Threads nlohmann_json ${FREETYPE_LIBRARIES})
target_include_directories(ors PUBLIC ${FREETYPE_INCLUDE_DIRS})
#target_link_libraries(pic2video PRIVATE ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${AVDEVICE_LIBRARY})