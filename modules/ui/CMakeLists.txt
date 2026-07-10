project(ui LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# DEPENDENCIES OF ui

# generic

# Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

#
# Source files.
#

# cefsimple sources.

set(CEFSIMPLE_SRCS src/simple_app.cc src/ui.cc include/simple_app.h
                   src/simple_handler.cc include/simple_handler.h)
set(CEFSIMPLE_SRCS_LINUX src/simple_handler_linux.cc)
set(CEFSIMPLE_SRCS_MAC src/cefsimple_mac.mm src/simple_handler_mac.mm)
set(CEFSIMPLE_SRCS_WINDOWS src/cefsimple_win.cc include/resource.h
                           src/simple_handler_win.cc)
append_platform_sources(CEFSIMPLE_SRCS)
source_group(cefsimple FILES ${CEFSIMPLE_SRCS})

set(CEFSIMPLE_WIN_SRCS_WINDOWS win/cefsimple.rc)
append_platform_sources(CEFSIMPLE_WIN_SRCS)
source_group(cefsimple\\\\win FILES ${CEFSIMPLE_WIN_SRCS})

set(CEFSIMPLE_SRCS ${CEFSIMPLE_SRCS} ${CEFSIMPLE_WIN_SRCS})

# cefsimple helper sources.
set(CEFSIMPLE_HELPER_SRCS_MAC process_helper_mac.cc)
append_platform_sources(CEFSIMPLE_HELPER_SRCS)
source_group(cefsimple FILES ${CEFSIMPLE_HELPER_SRCS})

# cefsimple resources.
set(CEFSIMPLE_RESOURCES_MAC_SRCS_MAC mac/Info.plist.in mac/cefsimple.icns)
append_platform_sources(CEFSIMPLE_RESOURCES_MAC_SRCS)
source_group(cefsimple\\\\mac FILES ${CEFSIMPLE_RESOURCES_MAC_SRCS})

set(CEFSIMPLE_RESOURCES_MAC_ENGLISH_LPROJ_SRCS_MAC
    mac/English.lproj/InfoPlist.strings mac/English.lproj/MainMenu.xib)
append_platform_sources(CEFSIMPLE_RESOURCES_MAC_ENGLISH_LPROJ_SRCS)
source_group(cefsimple\\\\mac\\\\English.lproj
             FILES ${CEFSIMPLE_RESOURCES_MAC_ENGLISH_LPROJ_SRCS})

set(CEFSIMPLE_RESOURCES_SRCS ${CEFSIMPLE_RESOURCES_MAC_SRCS}
                             ${CEFSIMPLE_RESOURCES_MAC_ENGLISH_LPROJ_SRCS})

#
# Shared configuration.
#

# TODO: CHANGE TARGET TO LIBRARY (LINUX:DONE WINDOWS:NOT DONE MAC:NOT DONE)

# Target executable names.
set(CEF_TARGET "ui")

if(OS_MAC)
  set(CEF_HELPER_TARGET "pilot_Helper")
  set(CEF_HELPER_OUTPUT_NAME "pilot Helper")
else()
  # Logical target used to link the libcef library.
  add_logical_target("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")
endif()

# Determine the target output directory. set_cef_target_out_dir()

#
# Linux configuration.
#

if(OS_LINUX)
  add_library(${CEF_TARGET} STATIC ${CEFSIMPLE_SRCS})

  add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
  target_link_libraries(${CEF_TARGET} PRIVATE libcef_lib libcef_dll_wrapper
                                              ${CEF_STANDARD_LIBS} generic)

  # Set rpath so that libraries can be placed next to the executable.
  # set_target_properties(${CEF_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN")
  # set_target_properties(${CEF_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH
  # TRUE) set_target_properties(${CEF_TARGET} PROPERTIES
  # RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR})

  # copy_files("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}"
  # "${CEF_TARGET_OUT_DIR}")
  # copy_files("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}"
  # "${CEF_TARGET_OUT_DIR}")
  # if(EXISTS "${CEF_BINARY_DIR}/libminigbm.so") copy_files("${CEF_TARGET}"
  # "libminigbm.so" "${CEF_BINARY_DIR}"
  # "${CEF_TARGET_OUT_DIR}")
  # endif()

  target_include_directories(${CEF_TARGET} PRIVATE "${CEF_ROOT}")

  target_include_directories(${CEF_TARGET}
                             PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

  # TODO: TEMPORAL FOR TESTING add_executable(test
  # "${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp") target_link_libraries(test
  # PRIVATE ${CEF_TARGET}) target_include_directories(test PRIVATE
  # "${CMAKE_CURRENT_SOURCE_DIR}/include" ${CEF_ROOT})
  # set_target_properties(test PROPERTIES INSTALL_RPATH "$ORIGIN")
  # set_target_properties(test PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
  # set_target_properties(test PROPERTIES RUNTIME_OUTPUT_DIRECTORY
  # ${CEF_TARGET_OUT_DIR})

  # Set SUID permissions on the chrome-sandbox target.
  # set_linux_suid_permissions("${CEF_TARGET}"
  # "${CEF_TARGET_OUT_DIR}/chrome-sandbox") Executable target.
  # add_executable(${CEF_TARGET} ${CEFSIMPLE_SRCS})
  # set_executable_target_properties(${CEF_TARGET})
  # add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
  # target_link_libraries(${CEF_TARGET} PRIVATE libcef_lib libcef_dll_wrapper
  # ${CEF_STANDARD_LIBS})

  # Set rpath so that libraries can be placed next to the executable.
  # set_target_properties(${CEF_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN")
  # set_target_properties(${CEF_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH
  # TRUE) set_target_properties(${CEF_TARGET} PROPERTIES
  # RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR})

  # Copy binary and resource files to the target output directory.
  # copy_files("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}"
  # "${CEF_TARGET_OUT_DIR}")
  # copy_files("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}"
  # "${CEF_TARGET_OUT_DIR}")
  # if(EXISTS "${CEF_BINARY_DIR}/libminigbm.so") copy_files("${CEF_TARGET}"
  # "libminigbm.so" "${CEF_BINARY_DIR}"
  # "${CEF_TARGET_OUT_DIR}")
  # endif()

  # Set SUID permissions on the chrome-sandbox target.
  #set_linux_suid_permissions("${CEF_TARGET}"
  #                           "${CEF_TARGET_OUT_DIR}/chrome-sandbox")
endif()

#
# Mac OS X configuration.
#

if(OS_MAC)
  option(OPTION_USE_ARC
         "Build with ARC (automatic Reference Counting) on macOS." ON)
  if(OPTION_USE_ARC)
    list(APPEND CEF_COMPILER_FLAGS -fobjc-arc)
    set_target_properties(${target} PROPERTIES CLANG_ENABLE_OBJC_ARC "YES")
  endif()

  # Output path for the main app bundle.
  set(CEF_APP "${CEF_TARGET_OUT_DIR}/${CEF_TARGET}.app")

  # Variables referenced from the main Info.plist file.
  set(EXECUTABLE_NAME "${CEF_TARGET}")
  set(PRODUCT_NAME "${CEF_TARGET}")

  # Main app bundle target.
  add_executable(${CEF_TARGET} MACOSX_BUNDLE ${CEFSIMPLE_RESOURCES_SRCS}
                                             ${CEFSIMPLE_SRCS})
  set_executable_target_properties(${CEF_TARGET})
  add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
  target_link_libraries(${CEF_TARGET} libcef_dll_wrapper ${CEF_STANDARD_LIBS})
  set_target_properties(
    ${CEF_TARGET} PROPERTIES MACOSX_BUNDLE_INFO_PLIST
                             ${CMAKE_CURRENT_SOURCE_DIR}/mac/Info.plist.in)

  # Copy the CEF framework into the Frameworks directory and create the
  # necessary symlinks.
  copy_mac_framework("${CEF_TARGET}" "${CEF_BINARY_DIR}" "${CEF_APP}")

  # Create the multiple Helper app bundle targets.
  foreach(_suffix_list ${CEF_HELPER_APP_SUFFIXES})
    # Convert to a list and extract the suffix values.
    string(REPLACE ":" ";" _suffix_list ${_suffix_list})
    list(GET _suffix_list 0 _name_suffix)
    list(GET _suffix_list 1 _target_suffix)
    list(GET _suffix_list 2 _plist_suffix)

    # Define Helper target and output names.
    set(_helper_target "${CEF_HELPER_TARGET}${_target_suffix}")
    set(_helper_output_name "${CEF_HELPER_OUTPUT_NAME}${_name_suffix}")

    # Create Helper-specific variants of the helper-Info.plist file. Do this
    # manually because the configure_file command (which is executed as part of
    # MACOSX_BUNDLE_INFO_PLIST) uses global env variables and would insert the
    # wrong values with multiple targets.
    set(_helper_info_plist
        "${CMAKE_CURRENT_BINARY_DIR}/helper-Info${_target_suffix}.plist")
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/mac/helper-Info.plist.in"
         _plist_contents)
    string(REPLACE "\${EXECUTABLE_NAME}" "${_helper_output_name}"
                   _plist_contents ${_plist_contents})
    string(REPLACE "\${PRODUCT_NAME}" "${_helper_output_name}" _plist_contents
                   ${_plist_contents})
    string(REPLACE "\${BUNDLE_ID_SUFFIX}" "${_plist_suffix}" _plist_contents
                   ${_plist_contents})
    file(WRITE ${_helper_info_plist} ${_plist_contents})

    # Create Helper executable target.
    add_executable(${_helper_target} MACOSX_BUNDLE ${CEFSIMPLE_HELPER_SRCS})
    set_executable_target_properties(${_helper_target})
    add_dependencies(${_helper_target} libcef_dll_wrapper)
    target_link_libraries(${_helper_target} libcef_dll_wrapper
                          ${CEF_STANDARD_LIBS})
    set_target_properties(
      ${_helper_target}
      PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${_helper_info_plist}
                 OUTPUT_NAME ${_helper_output_name})

    # Add the Helper as a dependency of the main executable target.
    add_dependencies(${CEF_TARGET} "${_helper_target}")

    # Copy the Helper app bundle into the Frameworks directory.
    add_custom_command(
      TARGET ${CEF_TARGET}
      POST_BUILD
      COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${CEF_TARGET_OUT_DIR}/${_helper_output_name}.app"
        "${CEF_APP}/Contents/Frameworks/${_helper_output_name}.app"
      VERBATIM)
  endforeach()

  # Manually process and copy over resource files. The Xcode generator can
  # support this via the set_target_properties RESOURCE directive but that
  # doesn't properly handle nested resource directories. Remove these prefixes
  # from input file paths.
  set(PREFIXES "mac/")
  copy_mac_resources("${CEFSIMPLE_RESOURCES_SRCS}" "${PREFIXES}"
                     "${CEF_TARGET}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CEF_APP}")
endif()

#
# Windows configuration.
#

if(OS_WINDOWS)
  if(USE_SANDBOX)
    # Shared library (DLL) target.
    add_library(${CEF_TARGET} SHARED ${CEFSIMPLE_SRCS})
    set_library_target_properties(${CEF_TARGET})

    # Copy and rename the bootstrap binary.
    copy_single_file(${CEF_TARGET} ${CEF_BINARY_DIR}/bootstrap.exe
                     ${CEF_TARGET_OUT_DIR}/${CEF_TARGET}.exe)

    if(CMAKE_GENERATOR MATCHES "Visual Studio")
      # Make the bootstrap binary the default command for the Visual Studio
      # debugger.
      set_target_properties(
        ${CEF_TARGET}
        PROPERTIES VS_DEBUGGER_COMMAND
                   "$<TARGET_FILE_DIR:${CEF_TARGET}>/${CEF_TARGET}.exe")
    endif()
  else()
    # Executable target.
    add_executable(${CEF_TARGET} WIN32 ${CEFSIMPLE_SRCS})
    set_executable_target_properties(${CEF_TARGET})

    # Add the custom manifest files to the executable.
    add_windows_manifest("${CMAKE_CURRENT_SOURCE_DIR}/win" "${CEF_TARGET}"
                         "exe")
  endif()

  add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
  target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper
                        ${CEF_STANDARD_LIBS})

  # Copy binary and resource files to the target output directory.
  copy_files("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}"
             "${CEF_TARGET_OUT_DIR}")
  copy_files("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}"
             "${CEF_TARGET_OUT_DIR}")

  if(USE_SANDBOX)
    # Set LPAC ACLs required for Windows sandbox support.
    set_lpac_acls("${CEF_TARGET}")
  endif()
endif()
