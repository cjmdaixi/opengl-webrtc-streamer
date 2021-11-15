cmake_minimum_required(VERSION 3.17)
project(ocs)

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
        ocs
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/encoder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rtmp_publisher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/streamer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/scene.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rtc_publisher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dispatchqueue.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/fileparser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/h264fileparser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/opusfileparser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/stream.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/bufferparser.cpp
        ${GLAD_SRC}
)

find_package(Threads REQUIRED)
#target_include_directories(pic2video PRIVATE ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${AVDEVICE_INCLUDE_DIR})
target_link_libraries(ocs ${LIBS} PkgConfig::LIBAV LibDataChannel::LibDataChannel Threads::Threads nlohmann_json)
#target_link_libraries(pic2video PRIVATE ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${AVDEVICE_LIBRARY})