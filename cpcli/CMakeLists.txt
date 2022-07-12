# Proejct's sub diretory
add_subdirectory(utils)
add_subdirectory(operations)
add_subdirectory(path_manager)
add_subdirectory(cpcli)

# Checkers diretory
add_subdirectory(checkers)

set(TARGET_EXEC "cpcli_app")
add_executable(${TARGET_EXEC} main.cpp)
target_link_libraries(${TARGET_EXEC} cpcli_lib)

# Install to user local
install(TARGETS cpcli_app CONFIGURATIONS Release DESTINATION ~/.local/bin)

