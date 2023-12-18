add_executable(tests
  tests_ini.cpp tests_registry.cpp tests_utf8.cpp
  tests.rc
)

target_include_directories(tests PUBLIC ${PROJECT_SOURCE_DIR}/include)

# All link directories are subfolders of ./lib
target_link_directories (tests PUBLIC ${PROJECT_SOURCE_DIR}/lib/${pfx}/$<CONFIG>)

# Add dependent libraries
add_dependencies(tests utf8)
target_link_libraries (tests PRIVATE utf8.lib)
