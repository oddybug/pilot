# GLM

# Glad

# Glad is compiled and linked manualy. Note that IOM_SRC contains the src of
# glad.

# EGL


find_package(PkgConfig REQUIRED)
pkg_check_modules(EGL REQUIRED egl)

# lib src
set(RENDERER_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/render.c"
    # "${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/shader.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/texture.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entity.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/camera.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/object.c")

add_library(render ${RENDERER_SRC})

target_include_directories(render PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_include_directories(render PRIVATE "${THIRD_PARTY_DIR}/glad/include"
                                          "${EGL_INCLUDE_DIRS}")

target_link_libraries(render PRIVATE generic GL ${EGL_LIBRARIES}
                                     cglm assimp)
