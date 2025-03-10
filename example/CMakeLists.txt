cmake_minimum_required(VERSION 3.14)

project(geri-smx-decoderExamples CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(geri-smx-decoder REQUIRED)
endif()

add_custom_target(run-examples)

function(add_example NAME)
  add_executable("${NAME}" "${NAME}.cpp")
  target_link_libraries("${NAME}" PRIVATE geri-smx-decoder::geri-smx-decoder)
  add_custom_target("run_${NAME}" COMMAND "${NAME}" VERBATIM)
  add_dependencies("run_${NAME}" "${NAME}")
  add_dependencies(run-examples "run_${NAME}")
endfunction()

add_example(file_read_example)
target_compile_features(file_read_example PRIVATE cxx_std_23)

add_example(file_read_example_cpp11)
target_compile_features(file_read_example_cpp11 PRIVATE cxx_std_11)

add_folders(Example)
