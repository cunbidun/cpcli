cmake_minimum_required(VERSION 3.2)

include(FetchContent)

# Use C++17
set(CMAKE_CXX_STANDARD 17)

# Project's name
project(CPCLI)

# Export the compile_command.json for include directive
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(INSTALL_GTEST OFF)
set(CROW_INSTALL OFF)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

# Set the lib and bin output
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Target
set(TEST_TARGET_NAME "cpcli_test")

# Golbal include diretory
include_directories(${CPCLI_SOURCE_DIR}/include)

if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  set(CMAKE_BUILD_TYPE "Debug")

  message("Building with CodeCoverage")

  include(CodeCoverage)

  set(COVERAGE_EXCLUDES
    "${CPCLI_SOURCE_DIR}/cpcli/checkers/*"
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

  # CPCLI's tests
  add_subdirectory(test)
endif()

# Getting Json 
FetchContent_Declare(
  json 
  URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.x
  GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
  crow 
  URL https://github.com/CrowCpp/Crow/archive/refs/tags/v1.0+4.tar.gz
)
FetchContent_MakeAvailable(crow)

FetchContent_Declare(
  cxxopts
  URL https://github.com/jarro2783/cxxopts/archive/refs/tags/v3.0.0.tar.gz
)
FetchContent_MakeAvailable(cxxopts)

# CPCLI's source code
add_subdirectory(cpcli)
