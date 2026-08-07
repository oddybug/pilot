set(WEBSITE_DIR "${CMAKE_SOURCE_DIR}/ui")
set(WEBSITE_DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/ui")

file(
  GLOB_RECURSE WEBSITE_FILES
  RELATIVE "${WEBSITE_DIR}"
  "${WEBSITE_DIR}/*")

set(OUTPUT_FILES "")
foreach(FILE ${WEBSITE_FILES})
  set(SRC_FILE "${WEBSITE_DIR}/${FILE}")
  set(DST_FILE "${WEBSITE_DEST_DIR}/${FILE}")

  add_custom_command(
    OUTPUT "${DST_FILE}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SRC_FILE}" "${DST_FILE}"
    DEPENDS "${SRC_FILE}"
    COMMENT "Updating resource: ${FILE} from: ${WEBSITE_DIR} to:
    ${WEBSITE_DEST_DIR}")
  list(APPEND OUTPUT_FILES "${DST_FILE}")
endforeach()

add_custom_target(copy_app_website ALL DEPENDS ${OUTPUT_FILES})

add_dependencies(pilot copy_app_website)

