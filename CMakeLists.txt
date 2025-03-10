cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    geri-smx-decoder
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES NONE
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(geri-smx-decoder_geri-smx-decoder INTERFACE)
add_library(geri-smx-decoder::geri-smx-decoder ALIAS geri-smx-decoder_geri-smx-decoder)

set_property(
    TARGET geri-smx-decoder_geri-smx-decoder PROPERTY
    EXPORT_NAME geri-smx-decoder
)

target_include_directories(
    geri-smx-decoder_geri-smx-decoder ${warning_guard}
    INTERFACE
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(geri-smx-decoder_geri-smx-decoder INTERFACE cxx_std_11)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${geri-smx-decoder_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    add_subdirectory(example)
  endif()
endif()

# ---- Developer mode ----

if(NOT geri-smx-decoder_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of geri-smx-decoder"
  )
endif()

include(cmake/dev-mode.cmake)
