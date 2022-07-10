set(TARGET_EXEC "cpcli_app")

add_executable(${TARGET_EXEC} main.cpp)

target_link_libraries(${TARGET_EXEC} nlohmann_json::nlohmann_json)
target_link_libraries(${TARGET_EXEC} spdlog::spdlog)
target_link_libraries(${TARGET_EXEC} utils_lib)
target_link_libraries(${TARGET_EXEC} operations_lib)

install(TARGETS ${TARGET_EXEC} DESTINATION ~/.local/bin)
