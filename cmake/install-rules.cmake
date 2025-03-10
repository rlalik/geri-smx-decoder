if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/geri-smx-decoder-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

# Project is configured with no languages, so tell GNUInstallDirs the lib dir
set(CMAKE_INSTALL_LIBDIR lib CACHE PATH "")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package geri-smx-decoder)

install(
    DIRECTORY include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT geri-smx-decoder_Development
)

install(
    TARGETS geri-smx-decoder_geri-smx-decoder
    EXPORT geri-smx-decoderTargets
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

# Allow package maintainers to freely override the path for the configs
set(
    geri-smx-decoder_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE geri-smx-decoder_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(geri-smx-decoder_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${geri-smx-decoder_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT geri-smx-decoder_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${geri-smx-decoder_INSTALL_CMAKEDIR}"
    COMPONENT geri-smx-decoder_Development
)

install(
    EXPORT geri-smx-decoderTargets
    NAMESPACE geri-smx-decoder::
    DESTINATION "${geri-smx-decoder_INSTALL_CMAKEDIR}"
    COMPONENT geri-smx-decoder_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
