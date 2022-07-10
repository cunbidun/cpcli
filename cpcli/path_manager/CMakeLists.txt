set(TARGET_LIB "path_manager_lib")

add_library(${TARGET_LIB} path_manager.cpp)

target_link_libraries(${TARGET_LIB} nlohmann_json::nlohmann_json)
target_link_libraries(${TARGET_LIB} spdlog::spdlog)
