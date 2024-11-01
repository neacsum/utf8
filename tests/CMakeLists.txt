add_executable(tests
  tests_ini.cpp tests_win.cpp tests_utf8.cpp
  tests.rc
)

target_include_directories(tests PUBLIC ${PROJECT_SOURCE_DIR}/include)
set_property(TARGET tests PROPERTY CXX_STANDARD 17)

# All link directories are subfolders of ./lib
target_link_directories (tests PUBLIC ${PROJECT_SOURCE_DIR}/lib/${pfx}/$<CONFIG>)

# Add dependent libraries
add_dependencies(tests utf8)
target_link_libraries (tests PRIVATE utf8)
