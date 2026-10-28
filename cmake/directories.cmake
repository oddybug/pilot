# This file adds preprocessor macros to deal with directories

# TODO: when building for windows or for a package put absolute directories for
# where the package will actualy install. Same for cmakes from copiing files
# (see pilot cmake scripts) !URGENT

if(PILOT_USE_INSTALL_PATHS)
  include(GNUInstallDirs)

  target_compile_definitions(
    pilot
    PRIVATE
      SHADERS_SOURCE_DIR="${CMAKE_INSTALL_FULL_DATADIR}/pilot/resources/shaders/"
  )

  target_compile_definitions(
    pilot
    PRIVATE
      TEXTURES_SOURCE_DIR="${CMAKE_INSTALL_FULL_DATADIR}/pilot/resources/textures/"
  )

  target_compile_definitions(
    render
    PRIVATE
      SHADERS_SOURCE_DIR="${CMAKE_INSTALL_FULL_DATADIR}/pilot/resources/shaders/"
  )

  target_compile_definitions(
    render
    PRIVATE
      TEXTURES_SOURCE_DIR="${CMAKE_INSTALL_FULL_DATADIR}/pilot/resources/textures/"
  )

  target_compile_definitions(
    ui PRIVATE WEBSITE_SOURCE_DIR="${CMAKE_INSTALL_FULL_DATADIR}/pilot/ui/")

  target_compile_definitions(
    pilot PRIVATE WEBSITE_SOURCE_DIR="${CMAKE_INSTALL_FULL_DATADIR}/pilot/ui/")
else()
  message("Printing shader directory for module renderer:")
  message("${CMAKE_CURRENT_BINARY_DIR}/pilot/resources/shaders/")

  target_compile_definitions(
    pilot
    PRIVATE
      SHADERS_SOURCE_DIR="${CMAKE_CURRENT_BINARY_DIR}/pilot/resources/shaders/")

  target_compile_definitions(
    pilot
    PRIVATE
      TEXTURES_SOURCE_DIR="${CMAKE_CURRENT_BINARY_DIR}/pilot/resources/textures/"
  )

  target_compile_definitions(
    pilot PRIVATE WEBSITE_SOURCE_DIR="${CMAKE_CURRENT_BINARY_DIR}/pilot/ui/")

  target_compile_definitions(
    render
    PRIVATE
      SHADERS_SOURCE_DIR="${CMAKE_CURRENT_BINARY_DIR}/pilot/resources/shaders/")

  target_compile_definitions(
    render
    PRIVATE
      TEXTURES_SOURCE_DIR="${CMAKE_CURRENT_BINARY_DIR}/pilot/resources/textures/"
  )

  target_compile_definitions(
    ui PRIVATE WEBSITE_SOURCE_DIR="${CMAKE_CURRENT_BINARY_DIR}/pilot/ui/")
endif()
