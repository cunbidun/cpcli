# Get googletest from github
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Build test executable
enable_testing()


add_executable(${TEST_TARGET_NAME} test_main.cpp test_util.cpp)
target_link_libraries(${TEST_TARGET_NAME} utils_lib)
target_link_libraries(${TEST_TARGET_NAME} operations_lib)
target_link_libraries(${TEST_TARGET_NAME} gtest_main)

include(GoogleTest)
gtest_discover_tests(${TEST_TARGET_NAME})
