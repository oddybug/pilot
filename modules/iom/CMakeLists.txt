project(iom LANGUAGES C)

# dependecies

# SDL
add_subdirectory(${THIRD_PARTY_DIR}/SDL3 "${CMAKE_CURRENT_BINARY_DIR}/SDL3"
                 EXCLUDE_FROM_ALL)

# Glad

# Glad is compiled and linked manualy. Note that IOM_SRC contains the src of
# glad.

# EGL

find_package(PkgConfig REQUIRED)
pkg_check_modules(EGL REQUIRED egl)

# lib src
set(IOM_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/io_manager.c"
    "${THIRD_PARTY_DIR}/glad/src/egl.c" "${THIRD_PARTY_DIR}/glad/src/gl.c")

add_library(iom STATIC ${IOM_SRC})

target_include_directories(iom PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_include_directories(iom PRIVATE "${THIRD_PARTY_DIR}/glad/include"
                                       "${EGL_INCLUDE_DIRS}")

target_link_libraries(iom PRIVATE generic SDL3::SDL3 GL ${EGL_LIBRARIES})
