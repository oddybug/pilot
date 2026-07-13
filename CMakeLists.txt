cmake_minimum_required(VERSION 4.3)

# Project name. TODO: Change this line to match your project name when you copy
# this file.
project(pilot LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use folders in the resulting project files.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

#
# CEF_ROOT setup. This variable must be set to locate the binary distribution.
# TODO: Choose one of the below examples and comment out the rest.
#

# Example 1: The current directory contains both the complete binary
# distribution and your project. A. Comment in these lines:
#
set(CEF_ROOT "${THIRD_PARTY_DIR}/cef")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

# Example 2: The binary distribution is in a separate directory from your
# project. Locate the binary distribution using the CEF_ROOT CMake variable. A.
# Create a directory structure for your project like the following: myproject/
# CMakeLists.txt    <= top-level CMake configuration mytarget/ CMakeLists.txt <=
# CMake configuration for `mytarget` ... other `mytarget` source files B. Copy
# this file to "myproject/CMakeLists.txt" as the top-level CMake configuration.
# C. Create the target-specific "myproject/mytarget/CMakeLists.txt" file for
# your application. See the included cefclient and cefsimple CMakeLists.txt
# files as an example. D. Comment in these lines:
#
# set(CEF_ROOT "c:/path/to/cef_binary_3.2704.xxxx.gyyyyyyy_windows32")
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

# Example 3: The binary distribution is in a separate directory from your
# project. Locate the binary distribution using the CEF_ROOT environment
# variable. A. Create a directory structure for your project like the following:
# myproject/ CMakeLists.txt    <= top-level CMake configuration cmake/
# FindCEF.cmake   <= CEF CMake configuration entry point mytarget/
# CMakeLists.txt  <= CMake configuration for `mytarget` ... other `mytarget`
# source files B. Copy this file to "myproject/CMakeLists.txt" as the top-level
# CMake configuration. C. Copy the cmake/FindCEF.cmake file to
# "myproject/cmake/FindCEF.cmake". D. Create the target-specific
# "myproject/mytarget/CMakeLists.txt" file for your application. See the
# included cefclient and cefsimple CMakeLists.txt files as an example. E. Set
# the CEF_ROOT environment variable before executing CMake. For example: > set
# CEF_ROOT=c:\path\to\cef_binary_3.2704.xxxx.gyyyyyyy_windows32 F. Comment in
# these lines:
#
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
# "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#
# Load the CEF configuration.
#

# Execute FindCEF.cmake which must exist in CMAKE_MODULE_PATH.
find_package(CEF REQUIRED)

#
# Define CEF-based targets.
#

# Include the libcef_dll_wrapper target. Comes from the
# libcef_dll/CMakeLists.txt file in the binary distribution directory.
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pilot")
  add_subdirectory(pilot)
endif()

# Include application targets. Comes from the <target>/CMakeLists.txt file in
# the current directory. TODO: Change these lines to match your project target
# when you copy this file.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
  add_subdirectory(tests/cefsimple)
  add_subdirectory(tests/cefsimple_capi)
  add_subdirectory(tests/gmock)
  add_subdirectory(tests/gtest)
  add_subdirectory(tests/ceftests)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/cefclient")
  # add_subdirectory(tests/cefclient)
endif()

# Display configuration settings.
print_cef_config()

#
# Define the API documentation target.
#

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    add_custom_target(
      apidocs ALL
      # Generate documentation in the docs/html directory.
      COMMAND "${DOXYGEN_EXECUTABLE}" Doxyfile
      # Write a docs/index.html file.
      COMMAND ${CMAKE_COMMAND} -E make_directory docs
      COMMAND
        ${CMAKE_COMMAND} -E echo
        "<html><head><meta http-equiv=\"refresh\" content=\"0;URL='html/index.html'\"/></head></html>"
        > docs/index.html
      WORKING_DIRECTORY "${CEF_ROOT}"
      COMMENT "Generating API documentation with Doxygen..."
      USES_TERMINAL VERBATIM)
  else()
    message(WARNING "Doxygen must be installed to generate API documentation.")
  endif()
endif()

add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/cglm")

#add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/assimp")

add_subdirectory("${CMAKE_SOURCE_DIR}/modules/generic")

add_subdirectory("${CMAKE_SOURCE_DIR}/modules/iom")

add_subdirectory("${CMAKE_SOURCE_DIR}/modules/render")

add_subdirectory("${CMAKE_SOURCE_DIR}/modules/ui")

