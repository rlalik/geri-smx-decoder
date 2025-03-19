cmake_minimum_required(VERSION 3.14)

project(geri-smx-decoderTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----

include(FetchContent)

set(INSTALL_GTEST OFF)
FetchContent_Declare(GTest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(GTest)

if(PROJECT_IS_TOP_LEVEL)
  find_package(geri-smx-decoder REQUIRED)
  enable_testing()
endif()

# ---- Tests ----

add_executable(geri-smx-decoder_test source/geri-smx-decoder_test.cpp)
target_link_libraries(geri-smx-decoder_test PRIVATE geri-smx-decoder::geri-smx-decoder GTest::gtest_main)
target_compile_features(geri-smx-decoder_test PRIVATE cxx_std_23)

add_test(NAME geri-smx-decoder_test COMMAND geri-smx-decoder_test)

# ---- End-of-file commands ----

add_folders(Test)
