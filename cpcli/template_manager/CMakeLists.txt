set(TARGET_LIB "template_manager_lib")

add_library(${TARGET_LIB} template_manager.cpp)

target_link_libraries(${TARGET_LIB} nlohmann_json::nlohmann_json)
target_link_libraries(${TARGET_LIB} spdlog::spdlog)
