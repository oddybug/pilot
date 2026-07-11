# SDL3 add_subdirectory(${THIRD_PARTY_DIR}/SDL3
# "${CMAKE_CURRENT_BINARY_DIR}/SDL3" EXCLUDE_FROM_ALL)

# GLM add_subdirectory(${THIRD_PARTY_DIR}/glm "${CMAKE_CURRENT_BINARY_DIR}/glm"
# EXCLUDE_FROM_ALL)

# EGL find_package(PkgConfig REQUIRED) pkg_check_modules(EGL REQUIRED egl)

# DEMO
file(GLOB_RECURSE APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/global.c")

add_executable(pilot "${APP_SOURCES}")

# target_sources(pilot PRIVATE "${APP_SOURCES}")

target_include_directories(pilot PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(pilot PRIVATE iom render generic cglm SDL3::SDL3 ui)

# CEF

set_cef_target_out_dir()

if(OS_LINUX)
  set_target_properties(pilot PROPERTIES SKIP_BUILD_RPATH FALSE
                                         BUILD_WITH_INSTALL_RPATH FALSE)

  set(PILOT_BUILD_PATHS "$<TARGET_FILE_DIR:cglm>" "$<TARGET_FILE_DIR:generic>"
                        "$<TARGET_FILE_DIR:SDL3::SDL3>" "${CEF_TARGET_OUT_DIR}")

  # 1. Pass the list directly to the property
  set_target_properties(pilot PROPERTIES BUILD_RPATH "${PILOT_BUILD_PATHS}")

  set_target_properties(pilot PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
  set(CMAKE_RELATIVE_RPATH ON)

  copy_files("pilot" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}"
             "${CEF_TARGET_OUT_DIR}")
  copy_files("pilot" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}"
             "${CEF_TARGET_OUT_DIR}")

  if(EXISTS "${CEF_BINARY_DIR}/libminigbm.so")
    copy_files("pilot" "libminigbm.so" "${CEF_BINARY_DIR}"
               "${CEF_TARGET_OUT_DIR}")
  endif()

  set_linux_suid_permissions(pilot "${CEF_TARGET_OUT_DIR}/chrome-sandbox")
endif()

include(cmake/directories.cmake)
include(cmake/resources.cmake)
