set(TARGET_LIB "cpcli_lib")

add_library(${TARGET_LIB} cpcli.cpp)

target_link_libraries(${TARGET_LIB} nlohmann_json::nlohmann_json)
target_link_libraries(${TARGET_LIB} spdlog::spdlog)
target_link_libraries(${TARGET_LIB} utils_lib)
target_link_libraries(${TARGET_LIB} operations_lib)

