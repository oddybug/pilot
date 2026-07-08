# This file adds preprocessor macros to deal with directories

message("Printing shader directory for module renderer:")
message("${CMAKE_CURRENT_BINARY_DIR}/resources/shaders/")
target_compile_definitions(
  cef
  PRIVATE SHADERS_SOURCE_DIR="${CMAKE_CURRENT_BINARY_DIR}/resources/shaders/")
