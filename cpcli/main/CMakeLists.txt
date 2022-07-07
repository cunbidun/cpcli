add_executable(cpcli_app main.cpp)

target_link_libraries(cpcli_app utils_lib)
target_link_libraries(cpcli_app operations_lib)

install(TARGETS cpcli_app DESTINATION ~/.local/bin)
