set(TARGET_LIB "operations_lib")

add_library(${TARGET_LIB} compile_headers.cpp operations.cpp parse_config.cpp print_usage.cpp)

target_link_libraries(${TARGET_LIB} nlohmann_json::nlohmann_json)
target_link_libraries(${TARGET_LIB} spdlog::spdlog)
target_link_libraries(${TARGET_LIB} template_manager_lib)
