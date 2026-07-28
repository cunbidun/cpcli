cmake_minimum_required(VERSION 3.20)

project(cpcli VERSION 0.6.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(CLI11 CONFIG REQUIRED)
find_package(Crow CONFIG REQUIRED)
find_package(inja CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(toml11 CONFIG REQUIRED)

add_library(cpcli_color INTERFACE)
target_include_directories(cpcli_color INTERFACE core/libs/color/include)

add_library(cpcli_constant INTERFACE)
target_include_directories(cpcli_constant INTERFACE core/libs/constant/include)

add_library(cpcli_path_manager core/libs/path_manager/src/path_manager.cpp)
target_include_directories(cpcli_path_manager PUBLIC core/libs/path_manager/include)
target_link_libraries(cpcli_path_manager
  PUBLIC cpcli_constant nlohmann_json::nlohmann_json spdlog::spdlog
)

add_library(cpcli_utils core/libs/utils/src/utils.cpp)
target_include_directories(cpcli_utils PUBLIC core/libs/utils/include)
target_link_libraries(cpcli_utils PUBLIC cpcli_color cpcli_constant spdlog::spdlog)

add_library(cpcli_template_manager core/libs/template_manager/src/template_manager.cpp)
target_include_directories(cpcli_template_manager PUBLIC core/libs/template_manager/include)
target_link_libraries(cpcli_template_manager
  PUBLIC cpcli_path_manager nlohmann_json::nlohmann_json pantor::inja spdlog::spdlog
)

add_library(cpcli_operations
  core/libs/operations/src/operations.cpp
  core/libs/operations/src/parse_args.cpp
  core/libs/operations/src/parse_config.cpp
)
target_include_directories(cpcli_operations PUBLIC core/libs/operations/include)
target_link_libraries(cpcli_operations
  PUBLIC
    cpcli_constant
    cpcli_template_manager
    cpcli_utils
    CLI11::CLI11
    nlohmann_json::nlohmann_json
    pantor::inja
    spdlog::spdlog
    toml11::toml11
)

add_library(cpcli_compiler
  core/libs/compiler/src/compiler.cpp
  core/libs/compiler/src/rust.cpp
)
target_include_directories(cpcli_compiler PUBLIC core/libs/compiler/include)
target_link_libraries(cpcli_compiler
  PUBLIC
    cpcli_operations
    cpcli_path_manager
    cpcli_utils
    nlohmann_json::nlohmann_json
    pantor::inja
    spdlog::spdlog
)

add_executable(cpcli_app core/bin/cpcli_app.cpp)
target_link_libraries(cpcli_app PRIVATE cpcli_compiler)

add_executable(cpcli_cc core/bin/cpcli_cc.cpp)
target_link_libraries(cpcli_cc
  PRIVATE cpcli_operations cpcli_template_manager Crow::Crow
)

add_executable(cli_task_editor default/task_editor/cli_task_editor/cli_task_editor.cpp)
target_link_libraries(cli_task_editor PRIVATE cpcli_operations cpcli_template_manager)

foreach(checker IN ITEMS token_checker double_4 double_6 double_9)
  add_executable(${checker} default/checkers/src/${checker}.cpp)
  target_include_directories(${checker} PRIVATE default/checkers/include)
endforeach()

if(BUILD_TESTING)
  find_package(GTest CONFIG REQUIRED)
  add_executable(cpcli_test
    core/test/src/test_compiler.cpp
    core/test/src/test_path_manager.cpp
    core/test/src/test_util.cpp
  )
  target_link_libraries(cpcli_test PRIVATE cpcli_compiler GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(cpcli_test)
endif()

install(TARGETS cpcli_app cpcli_cc RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS token_checker double_4 double_6 double_9
  RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/cpcli/checkers
)
install(TARGETS cli_task_editor
  RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/cpcli/task-editor
)

install(PROGRAMS
  default/backends/accel_sim/bootstrap.sh
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  RENAME cpcli_cuda_bootstrap
)
install(PROGRAMS
  default/backends/accel_sim/compile.sh
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  RENAME cpcli_cuda_compile
)
install(PROGRAMS
  default/backends/accel_sim/run.sh
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  RENAME cpcli_cuda_run
)
install(PROGRAMS
  default/backends/accel_sim/setup.sh
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  RENAME cpcli_cuda_setup
)

install(DIRECTORY
  default/templates/common
  default/templates/cpp
  default/templates/cu
  default/templates/java
  default/templates/py
  default/templates/rs
  DESTINATION ${CMAKE_INSTALL_DATADIR}/cpcli/templates
)
install(FILES default/backends/accel_sim/README.md
  DESTINATION ${CMAKE_INSTALL_DATADIR}/cpcli/backends/accel-sim
)

# A plain CMake install uses the CLI editor. The Nix package replaces these
# links with the separately built Tauri editor.
install(CODE "
  if(IS_ABSOLUTE \"${CMAKE_INSTALL_BINDIR}\")
    set(bin_dir \"\$ENV{DESTDIR}${CMAKE_INSTALL_BINDIR}\")
  else()
    set(bin_dir \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\")
  endif()
  if(IS_ABSOLUTE \"${CMAKE_INSTALL_DATADIR}\")
    set(data_dir \"\$ENV{DESTDIR}${CMAKE_INSTALL_DATADIR}\")
  else()
    set(data_dir \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}\")
  endif()
  file(CREATE_LINK cli_task_editor
    \"\${data_dir}/cpcli/task-editor/task-editor\"
    SYMBOLIC)
  file(CREATE_LINK ../share/cpcli/task-editor/cli_task_editor
    \"\${bin_dir}/cpcli_editor\"
    SYMBOLIC)
")
