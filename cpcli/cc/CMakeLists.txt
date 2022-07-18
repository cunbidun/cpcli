set(TARGET_EXEC "cpcli_cc")

add_executable(${TARGET_EXEC} cc.cpp)
target_link_libraries(${TARGET_EXEC} utils_lib)
target_link_libraries(${TARGET_EXEC} operations_lib)
target_link_libraries(${TARGET_EXEC} path_manager_lib)
target_link_libraries(${TARGET_EXEC} template_manager_lib)
target_link_libraries(${TARGET_EXEC} Crow::Crow)
target_link_libraries(${TARGET_EXEC} cxxopts)
target_link_libraries(${TARGET_EXEC} spdlog::spdlog)
target_link_libraries(${TARGET_EXEC} nlohmann_json::nlohmann_json)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  # Install to user local
  install(TARGETS cpcli_cc CONFIGURATIONS Release DESTINATION ~/.local/bin)
endif()

