include(GNUInstallDirs)

set(PILOT_LIBEXEC_SUBDIR "${CMAKE_INSTALL_LIBEXECDIR}/pilot")
set(PILOT_DATA_SUBDIR "${CMAKE_INSTALL_DATADIR}/pilot")

if(PILOT_USE_INSTALL_PATHS)
  set_target_properties(
    pilot
    PROPERTIES INSTALL_RPATH "$ORIGIN" BUILD_WITH_INSTALL_RPATH TRUE
               INSTALL_RPATH_USE_LINK_PATH FALSE)
endif()

install(
  TARGETS pilot
  RUNTIME DESTINATION "${PILOT_LIBEXEC_SUBDIR}")

install(
  TARGETS generic cglm
  RUNTIME DESTINATION "${PILOT_LIBEXEC_SUBDIR}"
  LIBRARY DESTINATION "${PILOT_LIBEXEC_SUBDIR}"
  ARCHIVE DESTINATION "${PILOT_LIBEXEC_SUBDIR}")

if(TARGET SDL3-shared)
  install(
    TARGETS SDL3-shared
    RUNTIME DESTINATION "${PILOT_LIBEXEC_SUBDIR}"
    LIBRARY DESTINATION "${PILOT_LIBEXEC_SUBDIR}"
    ARCHIVE DESTINATION "${PILOT_LIBEXEC_SUBDIR}")
endif()

foreach(_pilot_cef_file IN LISTS CEF_BINARY_FILES)
  if(IS_DIRECTORY "${CEF_BINARY_DIR}/${_pilot_cef_file}")
    install(DIRECTORY "${CEF_BINARY_DIR}/${_pilot_cef_file}"
            DESTINATION "${PILOT_LIBEXEC_SUBDIR}")
  else()
    install(PROGRAMS "${CEF_BINARY_DIR}/${_pilot_cef_file}"
            DESTINATION "${PILOT_LIBEXEC_SUBDIR}")
  endif()
endforeach()

foreach(_pilot_cef_file IN LISTS CEF_RESOURCE_FILES)
  if(IS_DIRECTORY "${CEF_RESOURCE_DIR}/${_pilot_cef_file}")
    install(DIRECTORY "${CEF_RESOURCE_DIR}/${_pilot_cef_file}"
            DESTINATION "${PILOT_LIBEXEC_SUBDIR}")
  else()
    install(FILES "${CEF_RESOURCE_DIR}/${_pilot_cef_file}"
            DESTINATION "${PILOT_LIBEXEC_SUBDIR}")
  endif()
endforeach()

if(EXISTS "${CEF_BINARY_DIR}/libminigbm.so")
  install(PROGRAMS "${CEF_BINARY_DIR}/libminigbm.so"
          DESTINATION "${PILOT_LIBEXEC_SUBDIR}")
endif()

install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources/"
        DESTINATION "${PILOT_DATA_SUBDIR}/resources")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/ui/"
        DESTINATION "${PILOT_DATA_SUBDIR}/ui")

configure_file("${CMAKE_SOURCE_DIR}/cmake/pilot-wrapper.in"
               "${CMAKE_CURRENT_BINARY_DIR}/pilot-wrapper" @ONLY)

install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/pilot-wrapper"
        DESTINATION "${CMAKE_INSTALL_BINDIR}" RENAME pilot)

configure_file("${CMAKE_SOURCE_DIR}/cmake/pilot.desktop.in"
               "${CMAKE_CURRENT_BINARY_DIR}/pilot.desktop" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/pilot.desktop"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")

install(FILES "${CMAKE_SOURCE_DIR}/ui/favicon.svg"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps"
        RENAME pilot.svg)
