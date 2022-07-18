# Proejct's sub diretory
add_subdirectory(utils)
add_subdirectory(operations)
add_subdirectory(path_manager)
add_subdirectory(template_manager)
add_subdirectory(cpcli)

# Checkers diretory
add_subdirectory(checkers)

# Compatitve Companion 
if (NOT SKIP_COMPILE_CC)
  add_subdirectory(cc)
endif()

set(TARGET_EXEC "cpcli_app")
add_executable(${TARGET_EXEC} main.cpp)
target_link_libraries(${TARGET_EXEC} cpcli_lib)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  # Install to user local
  install(TARGETS cpcli_app CONFIGURATIONS Release DESTINATION ~/.local/bin)
endif()

