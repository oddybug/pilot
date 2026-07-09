set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/resources")
set(RESOURCE_DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/resources")

file(
  GLOB_RECURSE RESOURCE_FILES
  RELATIVE "${RESOURCE_DIR}"
  "${RESOURCE_DIR}/*")

set(OUTPUT_FILES "")
foreach(FILE ${RESOURCE_FILES})
  set(SRC_FILE "${RESOURCE_DIR}/${FILE}")
  set(DST_FILE "${RESOURCE_DEST_DIR}/${FILE}")

  add_custom_command(
    OUTPUT "${DST_FILE}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC_FILE}" "${DST_FILE}"
    DEPENDS "${SRC_FILE}"
    COMMENT "Updating resource: ${FILE}")
  list(APPEND OUTPUT_FILES "${DST_FILE}")
endforeach()

add_custom_target(copy_app_resources ALL DEPENDS ${OUTPUT_FILES})

add_dependencies(pilot copy_app_resources)
