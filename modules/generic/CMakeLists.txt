# lib src
set(GENERIC_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/data/list.c"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/log.c")

add_library(generic SHARED ${GENERIC_SRC})

target_include_directories(generic PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
