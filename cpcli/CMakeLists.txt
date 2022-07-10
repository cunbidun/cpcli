# Proejct's sub diretory
add_subdirectory(utils)
add_subdirectory(operations)
add_subdirectory(path_manager)
add_subdirectory(main)

# Checkers diretory
add_subdirectory(checkers)

# Install to user local
install(TARGETS cpcli_app CONFIGURATIONS Release DESTINATION ~/.local/bin)

