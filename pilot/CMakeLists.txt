# SDL3 add_subdirectory(${THIRD_PARTY_DIR}/SDL3
# "${CMAKE_CURRENT_BINARY_DIR}/SDL3" EXCLUDE_FROM_ALL)

# GLM add_subdirectory(${THIRD_PARTY_DIR}/glm "${CMAKE_CURRENT_BINARY_DIR}/glm"
# EXCLUDE_FROM_ALL)

# EGL find_package(PkgConfig REQUIRED) pkg_check_modules(EGL REQUIRED egl)

add_subdirectory("${CMAKE_SOURCE_DIR}/modules/iom"
                 "${CMAKE_CURRENT_BINARY_DIR}/modules/iom")

add_subdirectory("${CMAKE_SOURCE_DIR}/modules/render"
                 "${CMAKE_CURRENT_BINARY_DIR}/modules/render")

add_subdirectory("${CMAKE_SOURCE_DIR}/modules/ui"
                 "${CMAKE_CURRENT_BINARY_DIR}/modules/ui")

# DEMO
file(GLOB_RECURSE APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
     "${THIRD_PARTY_DIR}/Debug.hpp")

add_executable(pilot main.cpp)

target_sources(pilot PRIVATE "${APP_SOURCES}")

target_include_directories(pilot PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(pilot PRIVATE iom render generic cglm SDL3::SDL3 ui)

# CEF

if(OS_LINUX)
  set_target_properties(pilot PROPERTIES INSTALL_RPATH "\\$ORIGIN")
  set_target_properties(pilot PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)

  set_target_properties(pilot PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                         "${CEF_TARGET_OUT_DIR}")

  copy_files("pilot" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}"
             "${CEF_TARGET_OUT_DIR}")
  copy_files("pilot" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}"
             "${CEF_TARGET_OUT_DIR}")

  if(EXISTS "${CEF_BINARY_DIR}/libminigbm.so")
    copy_files("pilot" "libminigbm.so" "${CEF_BINARY_DIR}"
               "${CEF_TARGET_OUT_DIR}")
  endif()
endif()

include(cmake/directories.cmake)
include(cmake/resources.cmake)

# set_executable_target_properties(sdl_test)
