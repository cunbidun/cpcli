cmake_minimum_required(VERSION 3.2)

# Use C++17
set(CMAKE_CXX_STANDARD 17)

# Project's name
project(CPCLI)

# Export the compile_command.json for include directive
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(INSTALL_GTEST OFF)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

# Set the lib and bin output
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Target
set(TEST_TARGET_NAME "cpcli_test")

# Competitve companion handler
add_custom_target(competitive_companion 
    WORKING_DIRECTORY ${CPCLI_SOURCE_DIR}/cc
    COMMENT "Current working diretory is: ${PROJECT_SOURCE_DIR}/cc. Running 'npm install'..."
    COMMAND npm install
)

if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  set(CMAKE_BUILD_TYPE "Debug")

  message("Building with CodeCoverage")

  include(CodeCoverage)

  set(COVERAGE_EXCLUDES
    "${CPCLI_SOURCE_DIR}/cpcli/checkers/*"
    "${CPCLI_SOURCE_DIR}/third_party/*"
    "${CPCLI_SOURCE_DIR}/build/*"
    "${CPCLI_SOURCE_DIR}/test/*"
    "/Applications/*"
  )

  message("CPCLI_SOURCE_DIR is ${CPCLI_SOURCE_DIR}")
  message("COVERAGE_EXCLUDES is ${COVERAGE_EXCLUDES}")

  append_coverage_compiler_flags()

  setup_target_for_coverage_lcov(
    NAME coverage
    EXECUTABLE ${TEST_TARGET_NAME} 
  )
endif()

# Golbal include diretory
include_directories(${CPCLI_SOURCE_DIR}/include)
include_directories(${CPCLI_SOURCE_DIR}/third_party)

# CPCLI's source code
add_subdirectory(cpcli)

# CPCLI's tests
add_subdirectory(test)

