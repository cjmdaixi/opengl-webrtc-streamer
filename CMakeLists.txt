cmake_minimum_required(VERSION 3.17)
project(pic2video)

set(CMAKE_CXX_STANDARD 11)
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
add_subdirectory(3rd_party/glfw)
include_directories(3rd_party/glfw/include)
LIST(APPEND LIBS glfw ${GLFW_LIBRARIES})

include_directories(3rd_party/glad/include)
set(GLAD_SRC 3rd_party/glad/src/glad.c)
if(NOT WIN32)
    LIST(APPEND LIBS dl)
endif()

include_directories(3rd_party/glm)

add_executable(
        pic2video main.cpp ${GLAD_SRC}
)
target_link_libraries(pic2video  ${LIBS} PkgConfig::LIBAV )