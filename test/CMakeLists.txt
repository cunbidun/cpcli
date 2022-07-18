# Get googletest from github
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)


# Build test executable
enable_testing()

add_executable(${TEST_TARGET_NAME} test_main.cpp test_util.cpp test_path_manager.cpp)
target_link_libraries(${TEST_TARGET_NAME} spdlog::spdlog nlohmann_json::nlohmann_json)
target_link_libraries(${TEST_TARGET_NAME} utils_lib operations_lib path_manager_lib template_manager_lib cpcli_lib)
target_link_libraries(${TEST_TARGET_NAME} gtest_main)

include(GoogleTest)
gtest_discover_tests(${TEST_TARGET_NAME})
